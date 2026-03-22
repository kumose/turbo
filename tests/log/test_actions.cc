//
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

#include <tests/log/test_actions.h>

#include <cassert>
#include <iostream>
#include <string>
#include <type_traits>

#include <turbo/base/macros.h>
#include <turbo/strings/escaping.h>
#include <turbo/strings/str_cat.h>
#include <turbo/strings/string_view.h>
#include <turbo/times/time.h>

namespace turbo {
TURBO_NAMESPACE_BEGIN
namespace log_internal {

void WriteToStderrWithFilename::operator()(const turbo::LogEntry& entry) const {
  std::cerr << message << " (file: " << entry.source_filename() << ")\n";
}

void WriteEntryToStderr::operator()(const turbo::LogEntry& entry) const {
  if (!message.empty()) std::cerr << message << "\n";

  const std::string source_filename = turbo::c_hex_encode(entry.source_filename());
  const std::string source_basename = turbo::c_hex_encode(entry.source_basename());
  const std::string text_message = turbo::c_hex_encode(entry.text_message());
  const std::string encoded_message = turbo::c_hex_encode(entry.encoded_message());
  std::string encoded_message_str;
  std::cerr << "LogEntry{\n"                                               //
            << "  source_filename: \"" << source_filename << "\"\n"        //
            << "  source_basename: \"" << source_basename << "\"\n"        //
            << "  source_line: " << entry.source_line() << "\n"            //
            << "  prefix: " << (entry.prefix() ? "true\n" : "false\n")     //
            << "  log_severity: " << entry.log_severity() << "\n"          //
            << "  timestamp: " << entry.timestamp() << "\n"                //
            << "  text_message: \"" << text_message << "\"\n"              //
            << "  verbosity: " << entry.verbosity() << "\n"                //
            << "  encoded_message (raw): \"" << encoded_message << "\"\n"  //
            << encoded_message_str                                         //
            << "}\n";
}

void WriteEntryToStderr::operator()(turbo::LogSeverity severity,
                                    std::string_view filename,
                                    std::string_view log_message) const {
  if (!message.empty()) std::cerr << message << "\n";
  const std::string source_filename = turbo::c_hex_encode(filename);
  const std::string text_message = turbo::c_hex_encode(log_message);
  std::cerr << "LogEntry{\n"                                         //
            << "  source_filename: \"" << source_filename << "\"\n"  //
            << "  log_severity: " << severity << "\n"                //
            << "  text_message: \"" << text_message << "\"\n"        //
            << "}\n";
}

}  // namespace log_internal
TURBO_NAMESPACE_END
}  // namespace turbo
