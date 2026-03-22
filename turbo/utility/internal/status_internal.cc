// Copyright (C) 2026 Kumo inc. and its affiliates. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include <turbo/utility/internal/status_internal.h>

#include <atomic>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <memory>
#include <string>
#include <utility>

#include <turbo/base/macros.h>
#include <turbo/base/nullability.h>
#include <turbo/debugging/stacktrace.h>
#include <turbo/debugging/symbolize.h>
#include <turbo/memory/memory.h>
#include <turbo/utility/internal/status.h>
#include <turbo/utility/status_payload_printer.h>
#include <turbo/strings/cord.h>
#include <turbo/strings/escaping.h>
#include <turbo/strings/str_cat.h>
#include <turbo/strings/str_format.h>
#include <turbo/strings/str_split.h>
#include <turbo/strings/string_view.h>
#include <optional>

namespace turbo::status_internal {

    void StatusRep::Unref() const {
        // Fast path: if ref==1, there is no need for a RefCountDec (since
        // this is the only reference and therefore no other thread is
        // allowed to be mucking with r).
        if (ref_.load(std::memory_order_acquire) == 1 ||
            ref_.fetch_sub(1, std::memory_order_acq_rel) - 1 == 0) {
            delete this;
        }
    }

    static std::optional<size_t> FindPayloadIndexByUrl(
            const Payloads *payloads, std::string_view type_url) {
        if (payloads == nullptr) return std::nullopt;

        for (size_t i = 0; i < payloads->size(); ++i) {
            if (std::string_view((*payloads)[i]->type_id()) == type_url) return i;
        }

        return std::nullopt;
    }

    const std::shared_ptr<StatusPayload> &StatusRep::get_payload(
            std::string_view type_url) const {
        static std::shared_ptr<StatusPayload> null_payload = nullptr;

        std::optional<size_t> index =
                status_internal::FindPayloadIndexByUrl(payloads_.get(), type_url);
        if (index.has_value()) return (*payloads_)[index.value()];

        return null_payload;
    }

    std::shared_ptr<StatusPayload> &StatusRep::get_payload(std::string_view type_url) {
        static std::shared_ptr<StatusPayload> null_payload = nullptr;

        std::optional<size_t> index =
                status_internal::FindPayloadIndexByUrl(payloads_.get(), type_url);
        if (index.has_value()) return (*payloads_)[index.value()];

        return null_payload;
    }

    void StatusRep::set_payload(std::shared_ptr<StatusPayload> payload) {
        if (payloads_ == nullptr) {
            payloads_ = turbo::make_unique<status_internal::Payloads>();
        }

        std::optional<size_t> index =
                status_internal::FindPayloadIndexByUrl(payloads_.get(), payload->type_id());
        if (index.has_value()) {
            (*payloads_)[index.value()] = std::move(payload);
            return;
        }

        payloads_->push_back({std::move(payload)});
    }

    StatusRep::EraseResult StatusRep::erase_payload(std::string_view type_url) {
        std::optional<size_t> index =
                status_internal::FindPayloadIndexByUrl(payloads_.get(), type_url);
        if (!index.has_value()) return {false, Status::PointerToRep(this)};
        payloads_->erase(payloads_->begin() + index.value());
        if (payloads_->empty() && message_.empty()) {
            // Special case: If this can be represented inlined, it MUST be inlined
            // (== depends on this behavior).
            EraseResult result = {true, Status::CodeToInlinedRep(code_)};
            Unref();
            return result;
        }
        return {true, Status::PointerToRep(this)};
    }

    void StatusRep::for_each_payload(
            turbo::FunctionRef<void(std::string_view, const std::shared_ptr<StatusPayload> &)> visitor)
    const {
        if (auto *payloads = payloads_.get()) {
            bool in_reverse =
                    payloads->size() > 1 && reinterpret_cast<uintptr_t>(payloads) % 13 > 6;

            for (size_t index = 0; index < payloads->size(); ++index) {
                const auto &elem =
                        (*payloads)[in_reverse ? payloads->size() - 1 - index : index];

#ifdef NDEBUG
                visitor(std::string_view(elem->type_id()), elem);
#else
                // In debug mode invalidate the type url to prevent users from relying on
                // this string lifetime.

                // NOLINTNEXTLINE intentional extra conversion to force temporary.
                visitor(std::string_view(elem->type_id()), elem);
#endif  // NDEBUG
            }
        }
    }

    std::string StatusRep::to_string(StatusToStringMode mode) const {
        std::string text;
        turbo::str_append(&text, turbo::status_code_to_string(code()), ": ", message());

        const bool with_payload = (mode & StatusToStringMode::kWithPayload) ==
                                  StatusToStringMode::kWithPayload;
        if (with_payload) {
            status_internal::StatusPayloadPrinter printer =
                    status_internal::GetStatusPayloadPrinter();
            this->for_each_payload([&](std::string_view type_url,
                                     const std::shared_ptr<StatusPayload> &payload) {
                std::optional<std::string> result;
                auto desc = payload->to_string();
                if (printer) result = printer(type_url, desc);
                turbo::str_append(
                        &text, " [", type_url, "='",
                        result.has_value() ? *result : turbo::c_hex_encode(desc),
                        "']");
            });
        }

        return text;
    }

    bool StatusRep::operator==(const StatusRep &other) const {
        assert(this != &other);
        if (code_ != other.code_) return false;
        if (message_ != other.message_) return false;
        const status_internal::Payloads *this_payloads = payloads_.get();
        const status_internal::Payloads *other_payloads = other.payloads_.get();

        const status_internal::Payloads no_payloads;
        const status_internal::Payloads *larger_payloads =
                this_payloads ? this_payloads : &no_payloads;
        const status_internal::Payloads *smaller_payloads =
                other_payloads ? other_payloads : &no_payloads;
        if (larger_payloads->size() < smaller_payloads->size()) {
            std::swap(larger_payloads, smaller_payloads);
        }
        if ((larger_payloads->size() - smaller_payloads->size()) > 1) return false;
        // Payloads can be ordered differently, so we can't just compare payload
        // vectors.
        for (const auto &payload: *larger_payloads) {

            bool found = false;
            for (const auto &other_payload: *smaller_payloads) {
                if (std::string_view(payload->type_id()) == std::string_view(other_payload->type_id())) {
                    if (*payload != *other_payload) {
                        return false;
                    }
                    found = true;
                    break;
                }
            }
            if (!found) {
                return false;
            }
        }
        return true;
    }

    turbo::Nonnull<StatusRep *> StatusRep::CloneAndUnref() const {
        // Optimization: no need to create a clone if we already have a refcount of 1.
        if (ref_.load(std::memory_order_acquire) == 1) {
            // All StatusRep instances are heap allocated and mutable, therefore this
            // const_cast will never cast away const from a stack instance.
            //
            // CloneAndUnref is the only method that doesn't involve an external cast to
            // get a mutable StatusRep* from the uintptr_t rep stored in Status.
            return const_cast<StatusRep *>(this);
        }
        std::unique_ptr<status_internal::Payloads> payloads;
        if (payloads_) {
            payloads = turbo::make_unique<status_internal::Payloads>(*payloads_);
        }
        auto *new_rep = new StatusRep(code_, message_, std::move(payloads));
        Unref();
        return new_rep;
    }

        // Convert canonical code to a value known to this binary.
    turbo::StatusCode MapToLocalCode(int value) {
        turbo::StatusCode code = static_cast<turbo::StatusCode>(value);
        switch (code) {
            case turbo::StatusCode::kOk:
            case turbo::StatusCode::kCancelled:
            case turbo::StatusCode::kUnknown:
            case turbo::StatusCode::kInvalidArgument:
            case turbo::StatusCode::kDeadlineExceeded:
            case turbo::StatusCode::kNotFound:
            case turbo::StatusCode::kAlreadyExists:
            case turbo::StatusCode::kPermissionDenied:
            case turbo::StatusCode::kResourceExhausted:
            case turbo::StatusCode::kFailedPrecondition:
            case turbo::StatusCode::kAborted:
            case turbo::StatusCode::kOutOfRange:
            case turbo::StatusCode::kUnimplemented:
            case turbo::StatusCode::kInternal:
            case turbo::StatusCode::kUnavailable:
            case turbo::StatusCode::kDataLoss:
            case turbo::StatusCode::kUnauthenticated:
            case turbo::StatusCode::kIOError:
                return code;
            default:
                return turbo::StatusCode::kUnknown;
        }
    }

    turbo::Nonnull<std::string *> MakeCheckFailString(
            turbo::Nonnull<const turbo::Status *> status,
            turbo::Nonnull<const char *> prefix) {
        return new std::string(
                turbo::str_cat(prefix, " (",
                               status->to_string(StatusToStringMode::kWithEverything), ")"));
    }

}  // namespace turbo::status_internal
