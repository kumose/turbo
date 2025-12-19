// Copyright (C) 2024 Kumo inc.
// Author: Jeff.li lijippy@163.com
// All rights reserved.
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#include <turbo/files/file_util.h>
#include <turbo/files/file.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>
#include <sys/mman.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#if defined(OS_MACOSX)
#include <AvailabilityMacros.h>
#elif !defined(OS_CHROMEOS) && defined(USE_GLIB)
#include <glib.h>  // for g_get_home_dir()
#endif

#include <fstream>
#include <turbo/log/logging.h>
#include <turbo/memory/singleton.h>
#include <turbo/base/internal/eintr_wrapper.h>
#include <turbo/threading/thread_restrictions.h>

#if !defined(OS_IOS)

#include <grp.h>

#endif

namespace turbo {

    namespace {

#if defined(OS_BSD) || defined(OS_MACOSX) || defined(OS_NACL)
        static int CallStat(const char *path, stat_wrapper_t *sb) {
          ThreadRestrictions::AssertIOAllowed();
          return stat(path, sb);
        }
        static int CallLstat(const char *path, stat_wrapper_t *sb) {
          ThreadRestrictions::AssertIOAllowed();
          return lstat(path, sb);
        }
#else  //  defined(OS_BSD) || defined(OS_MACOSX) || defined(OS_NACL)

        static int CallStat(const char *path, stat_wrapper_t *sb) {
            ThreadRestrictions::AssertIOAllowed();
            return stat64(path, sb);
        }

        static int CallLstat(const char *path, stat_wrapper_t *sb) {
            ThreadRestrictions::AssertIOAllowed();
            return lstat64(path, sb);
        }

#endif  // !(defined(OS_BSD) || defined(OS_MACOSX) || defined(OS_NACL))


        std::string TempFileName() {
#if defined(OS_MACOSX)
            return StringPrintf(".%s.XXXXXX", turbo::mac::BaseBundleID());
#endif

#if defined(GOOGLE_CHROME_BUILD)
            return std::string(".com.google.Chrome.XXXXXX");
#else
            return std::string(".tech.kumo.XXXXXX");
#endif
        }


    }  // namespace


    static turbo::Status CreateTemporaryDirInDirImpl(const turbo::FilePath &base_dir,
                                            const turbo::FilePath &name_tmpl_path,
                                            turbo::FilePath *new_dir) {
        ThreadRestrictions::AssertIOAllowed();  // For call to mkdtemp().
        auto name_tmpl = name_tmpl_path.string();
        DKCHECK(name_tmpl.find("XXXXXX") != std::string::npos)
                        << "Directory name template must contain \"XXXXXX\".";

        auto sub_dir = base_dir / name_tmpl;
        std::string sub_dir_string = sub_dir.string();

        // this should be OK since mkdtemp just replaces characters in place
        char *buffer = const_cast<char *>(sub_dir_string.c_str());
        char *dtemp = mkdtemp(buffer);
        if (!dtemp) {
            DKLOG(ERROR) << "mkdtemp";
            return io_error("mkdtemp");
        }
        *new_dir = turbo::FilePath(dtemp);
        return turbo::OkStatus();
    }

    turbo::Result<turbo::FilePath> create_temporary_dir_in_dir(const turbo::FilePath &base_dir,
                                     const turbo::FilePath &prefix) {
        std::string mkdtemp_template = prefix;
        mkdtemp_template.append("XXXXXX");
        turbo::FilePath new_dir;
        TURBO_RETURN_NOT_OK(CreateTemporaryDirInDirImpl(base_dir, mkdtemp_template, &new_dir));
        return new_dir;
    }

    turbo::Result<turbo::FilePath> create_new_temp_directory(const turbo::FilePath &prefix) {
        TURBO_MOVE_OR_RAISE(auto tempdir, turbo::temp_directory_path());
        turbo::FilePath new_temp_path;
        TURBO_RETURN_NOT_OK(CreateTemporaryDirInDirImpl(tempdir, TempFileName(), &new_temp_path));
        return new_temp_path;
    }


// NaCl doesn't implement system calls to open files directly.
#if !defined(OS_NACL)

    FILE *file_to_FILE(File file, const char *mode) {
        FILE *stream = fdopen(file.get_platform_file(), mode);
        if (stream)
            file.take_platform_file();
        return stream;
    }

#endif  // !defined(OS_NACL)

    // Gets the current working directory for the process.
    bool get_current_directory(turbo::FilePath *dir) {
        // getcwd can return ENOENT, which implies it checks against the disk.
        ThreadRestrictions::AssertIOAllowed();

        char system_buffer[PATH_MAX] = "";
        if (!getcwd(system_buffer, sizeof(system_buffer))) {
            KLOG(FATAL);
            return false;
        }
        *dir = turbo::FilePath(system_buffer);
        return true;
    }

    // Sets the current working directory for the process.
    bool set_current_directory(const turbo::FilePath &path) {
        ThreadRestrictions::AssertIOAllowed();
        int ret = chdir(path.string().c_str());
        return !ret;
    }

}  // namespace turbo
