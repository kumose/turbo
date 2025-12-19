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
#include <turbo/files/scoped_temp_dir.h>
#include <gtest/gtest.h>

using turbo::File;

TEST(FileTest, Create) {
    turbo::ScopedTempDir temp_dir;
    ASSERT_TRUE(temp_dir.create_unique_temp_dir().ok());
    auto file_path = temp_dir.path() / "create_file_1";

    {
        // Don't create a File at all.
        File file;
        EXPECT_FALSE(file.is_valid());
        EXPECT_EQ(turbo::File::FILE_ERROR_FAILED, file.error_details());

        File file2(turbo::File::FILE_ERROR_TOO_MANY_OPENED);
        EXPECT_FALSE(file2.is_valid());
        EXPECT_EQ(turbo::File::FILE_ERROR_TOO_MANY_OPENED, file2.error_details());
    }

    {
        // Open a file that doesn't exist.
        File file(file_path, turbo::File::FLAG_OPEN | turbo::File::FLAG_READ);
        EXPECT_FALSE(file.is_valid());
        EXPECT_EQ(turbo::File::FILE_ERROR_NOT_FOUND, file.error_details());
    }

    {
        // Open or create a file.
        File file(file_path, turbo::File::FLAG_OPEN_ALWAYS | turbo::File::FLAG_READ);
        EXPECT_TRUE(file.is_valid());
        EXPECT_TRUE(file.created());
        EXPECT_EQ(turbo::File::FILE_OK, file.error_details());
    }

    {
        // Open an existing file.
        File file(file_path, turbo::File::FLAG_OPEN | turbo::File::FLAG_READ);
        EXPECT_TRUE(file.is_valid());
        EXPECT_FALSE(file.created());
        EXPECT_EQ(turbo::File::FILE_OK, file.error_details());

        // This time verify closing the file.
        file.close();
        EXPECT_FALSE(file.is_valid());
    }

    {
        // Open an existing file through Initialize
        File file;
        file.initialize(file_path, turbo::File::FLAG_OPEN | turbo::File::FLAG_READ);
        EXPECT_TRUE(file.is_valid());
        EXPECT_FALSE(file.created());
        EXPECT_EQ(turbo::File::FILE_OK, file.error_details());

        // This time verify closing the file.
        file.close();
        EXPECT_FALSE(file.is_valid());
    }

    {
        // Create a file that exists.
        File file(file_path, turbo::File::FLAG_CREATE | turbo::File::FLAG_READ);
        EXPECT_FALSE(file.is_valid());
        EXPECT_FALSE(file.created());
        EXPECT_EQ(turbo::File::FILE_ERROR_EXISTS, file.error_details());
    }

    {
        // Create or overwrite a file.
        File file(file_path,
                  turbo::File::FLAG_CREATE_ALWAYS | turbo::File::FLAG_WRITE);
        EXPECT_TRUE(file.is_valid());
        EXPECT_TRUE(file.created());
        EXPECT_EQ(turbo::File::FILE_OK, file.error_details());
    }

    {
        // Create a delete-on-close file.
        file_path = temp_dir.path() / "create_file_2";
        File file(file_path,
                  turbo::File::FLAG_OPEN_ALWAYS | turbo::File::FLAG_READ |
                  turbo::File::FLAG_DELETE_ON_CLOSE);
        EXPECT_TRUE(file.is_valid());
        EXPECT_TRUE(file.created());
        EXPECT_EQ(turbo::File::FILE_OK, file.error_details());
    }
    auto rs = turbo::exists(file_path);
    ASSERT_TRUE(rs.ok());
    EXPECT_FALSE(rs.value_or_die());
}

TEST(FileTest, Async) {
    turbo::ScopedTempDir temp_dir;
    ASSERT_TRUE(temp_dir.create_unique_temp_dir().ok());
    turbo::FilePath file_path = temp_dir.path() / ("create_file");

    {
        File file(file_path, turbo::File::FLAG_OPEN_ALWAYS | turbo::File::FLAG_ASYNC);
        EXPECT_TRUE(file.is_valid());
        EXPECT_TRUE(file.async());
    }

    {
        File file(file_path, turbo::File::FLAG_OPEN_ALWAYS);
        EXPECT_TRUE(file.is_valid());
        EXPECT_FALSE(file.async());
    }
}

TEST(FileTest, DeleteOpenFile) {
    turbo::ScopedTempDir temp_dir;
    ASSERT_TRUE(temp_dir.create_unique_temp_dir().ok());
    turbo::FilePath file_path = temp_dir.path() / ("create_file_1");

    // Create a file.
    File file(file_path,
              turbo::File::FLAG_OPEN_ALWAYS | turbo::File::FLAG_READ |
              turbo::File::FLAG_SHARE_DELETE);
    EXPECT_TRUE(file.is_valid());
    EXPECT_TRUE(file.created());
    EXPECT_EQ(turbo::File::FILE_OK, file.error_details());

    // Open an existing file and mark it as delete on close.
    File same_file(file_path,
                   turbo::File::FLAG_OPEN | turbo::File::FLAG_DELETE_ON_CLOSE |
                   turbo::File::FLAG_READ);
    EXPECT_TRUE(file.is_valid());
    EXPECT_FALSE(same_file.created());
    EXPECT_EQ(turbo::File::FILE_OK, same_file.error_details());

    // Close both handles and check that the file is gone.
    file.close();
    same_file.close();
    auto rs = turbo::exists(file_path);
    ASSERT_TRUE(rs.ok());
    EXPECT_FALSE(rs.value_or_die());
}

TEST(FileTest, ReadWrite) {
    turbo::ScopedTempDir temp_dir;
    ASSERT_TRUE(temp_dir.create_unique_temp_dir().ok());
    turbo::FilePath file_path = temp_dir.path() / ("read_write_file");
    File file(file_path,
              turbo::File::FLAG_CREATE | turbo::File::FLAG_READ |
              turbo::File::FLAG_WRITE);
    ASSERT_TRUE(file.is_valid());

    char data_to_write[] = "test";
    const int kTestDataSize = 4;

    // write 0 bytes to the file.
    int bytes_written = file.write(0, data_to_write, 0);
    EXPECT_EQ(0, bytes_written);

    // write "test" to the file.
    bytes_written = file.write(0, data_to_write, kTestDataSize);
    EXPECT_EQ(kTestDataSize, bytes_written);

    // read from EOF.
    char data_read_1[32];
    int bytes_read = file.read(kTestDataSize, data_read_1, kTestDataSize);
    EXPECT_EQ(0, bytes_read);

    // read from somewhere in the middle of the file.
    const int kPartialReadOffset = 1;
    bytes_read = file.read(kPartialReadOffset, data_read_1, kTestDataSize);
    EXPECT_EQ(kTestDataSize - kPartialReadOffset, bytes_read);
    for (int i = 0; i < bytes_read; i++)
        EXPECT_EQ(data_to_write[i + kPartialReadOffset], data_read_1[i]);

    // read 0 bytes.
    bytes_read = file.read(0, data_read_1, 0);
    EXPECT_EQ(0, bytes_read);

    // read the entire file.
    bytes_read = file.read(0, data_read_1, kTestDataSize);
    EXPECT_EQ(kTestDataSize, bytes_read);
    for (int i = 0; i < bytes_read; i++)
        EXPECT_EQ(data_to_write[i], data_read_1[i]);

    // read again, but using the trivial native wrapper.
    bytes_read = file.read_no_best_effort(0, data_read_1, kTestDataSize);
    EXPECT_LE(bytes_read, kTestDataSize);
    for (int i = 0; i < bytes_read; i++)
        EXPECT_EQ(data_to_write[i], data_read_1[i]);

    // write past the end of the file.
    const int kOffsetBeyondEndOfFile = 10;
    const int kPartialWriteLength = 2;
    bytes_written = file.write(kOffsetBeyondEndOfFile,
                               data_to_write, kPartialWriteLength);
    EXPECT_EQ(kPartialWriteLength, bytes_written);

    // Make sure the file was extended.
    int64_t file_size = turbo::file_size(file_path).value_or_die();
    EXPECT_EQ(kOffsetBeyondEndOfFile + kPartialWriteLength, file_size);

    // Make sure the file was zero-padded.
    char data_read_2[32];
    bytes_read = file.read(0, data_read_2, static_cast<int>(file_size));
    EXPECT_EQ(file_size, bytes_read);
    for (int i = 0; i < kTestDataSize; i++)
        EXPECT_EQ(data_to_write[i], data_read_2[i]);
    for (int i = kTestDataSize; i < kOffsetBeyondEndOfFile; i++)
        EXPECT_EQ(0, data_read_2[i]);
    for (int i = kOffsetBeyondEndOfFile; i < file_size; i++)
        EXPECT_EQ(data_to_write[i - kOffsetBeyondEndOfFile], data_read_2[i]);
}

TEST(FileTest, Append) {
    turbo::ScopedTempDir temp_dir;
    ASSERT_TRUE(temp_dir.create_unique_temp_dir().ok());
    auto file_path = temp_dir.path() / ("append_file");
    File file(file_path, turbo::File::FLAG_CREATE | turbo::File::FLAG_APPEND);
    ASSERT_TRUE(file.is_valid());

    char data_to_write[] = "test";
    const int kTestDataSize = 4;

    KLOG(INFO) << "write 0 bytes to the file.";
    int bytes_written = file.write(0, data_to_write, 0);
    EXPECT_EQ(0, bytes_written);

    KLOG(INFO) << "write \"test\" to the file.";
    bytes_written = file.write(0, data_to_write, kTestDataSize);
    EXPECT_EQ(kTestDataSize, bytes_written);

    file.close();
    File file2(file_path,
               turbo::File::FLAG_OPEN | turbo::File::FLAG_READ |
               turbo::File::FLAG_APPEND);
    ASSERT_TRUE(file2.is_valid());

    KLOG(INFO) << "Test passing the file around.";
    file = file2.Pass();
    EXPECT_FALSE(file2.is_valid());
    ASSERT_TRUE(file.is_valid());

    char append_data_to_write[] = "78";
    const int kAppendDataSize = 2;
    KLOG(INFO) << "Append \"78\" to the file.";
    bytes_written = file.write(0, append_data_to_write, kAppendDataSize);
    EXPECT_EQ(kAppendDataSize, bytes_written);
    KLOG(INFO) << 2;
    // read the entire file.
    char data_read_1[32];
    int bytes_read = file.read(0, data_read_1,
                               kTestDataSize + kAppendDataSize);
    EXPECT_EQ(kTestDataSize + kAppendDataSize, bytes_read);
    for (int i = 0; i < kTestDataSize; i++)
        EXPECT_EQ(data_to_write[i], data_read_1[i]);
    for (int i = 0; i < kAppendDataSize; i++)
        EXPECT_EQ(append_data_to_write[i], data_read_1[kTestDataSize + i]);
}


TEST(FileTest, Length) {
    turbo::ScopedTempDir temp_dir;
    ASSERT_TRUE(temp_dir.create_unique_temp_dir().ok());
    auto file_path = temp_dir.path() / ("truncate_file");
    File file(file_path,
              turbo::File::FLAG_CREATE | turbo::File::FLAG_READ |
              turbo::File::FLAG_WRITE);
    ASSERT_TRUE(file.is_valid());
    EXPECT_EQ(0, file.get_length());

    // write "test" to the file.
    char data_to_write[] = "test";
    int kTestDataSize = 4;
    int bytes_written = file.write(0, data_to_write, kTestDataSize);
    EXPECT_EQ(kTestDataSize, bytes_written);

    // Extend the file.
    const int kExtendedFileLength = 10;
    EXPECT_TRUE(file.set_length(kExtendedFileLength));
    EXPECT_EQ(kExtendedFileLength, file.get_length());
    int64_t file_size = turbo::file_size(file_path).value_or_die();
    EXPECT_EQ(kExtendedFileLength, file_size);

    // Make sure the file was zero-padded.
    char data_read[32];
    int bytes_read = file.read(0, data_read, static_cast<int>(file_size));
    EXPECT_EQ(file_size, bytes_read);
    for (int i = 0; i < kTestDataSize; i++)
        EXPECT_EQ(data_to_write[i], data_read[i]);
    for (int i = kTestDataSize; i < file_size; i++)
        EXPECT_EQ(0, data_read[i]);

    // Truncate the file.
    const int kTruncatedFileLength = 2;
    EXPECT_TRUE(file.set_length(kTruncatedFileLength));
    EXPECT_EQ(kTruncatedFileLength, file.get_length());
    file_size = turbo::file_size(file_path).value_or_die();
    EXPECT_EQ(kTruncatedFileLength, file_size);

    // Make sure the file was truncated.
    bytes_read = file.read(0, data_read, kTestDataSize);
    EXPECT_EQ(file_size, bytes_read);
    for (int i = 0; i < file_size; i++)
        EXPECT_EQ(data_to_write[i], data_read[i]);
}

// Flakily fails: http://crbug.com/86494
#if defined(OS_ANDROID)
TEST(FileTest, TouchGetInfo) {
#else
TEST(FileTest, DISABLED_TouchGetInfo) {
#endif
    turbo::ScopedTempDir temp_dir;
    ASSERT_TRUE(temp_dir.create_unique_temp_dir().ok());
    File file(temp_dir.path() / ("touch_get_info_file"),
              turbo::File::FLAG_CREATE | turbo::File::FLAG_WRITE |
              turbo::File::FLAG_WRITE_ATTRIBUTES);
    ASSERT_TRUE(file.is_valid());

    // Get info for a newly created file.
    turbo::File::Info info;
    EXPECT_TRUE(file.get_info(&info));

    // Add 2 seconds to account for possible rounding errors on
    // filesystems that use a 1s or 2s timestamp granularity.
    auto now = turbo::Time::current_time() + turbo::Duration::seconds(2);
    EXPECT_EQ(0, info.size);
    EXPECT_FALSE(info.is_directory);
    EXPECT_FALSE(info.is_symbolic_link);
    EXPECT_LE(info.last_accessed, now);
    EXPECT_LE(info.last_modified, now);
    EXPECT_LE(info.creation_time, now);
    auto creation_time = info.creation_time;

    // write "test" to the file.
    char data[] = "test";
    const int kTestDataSize = 4;
    int bytes_written = file.write(0, data, kTestDataSize);
    EXPECT_EQ(kTestDataSize, bytes_written);

    // Change the last_accessed and last_modified dates.
    // It's best to add values that are multiples of 2 (in seconds)
    // to the current last_accessed and last_modified times, because
    // FATxx uses a 2s timestamp granularity.
    auto new_last_accessed =
            info.last_accessed + turbo::Duration::seconds(234);
    auto new_last_modified =
            info.last_modified + turbo::Duration::seconds(567);

    EXPECT_TRUE(file.set_times(new_last_accessed, new_last_modified));

    // Make sure the file info was updated accordingly.
    EXPECT_TRUE(file.get_info(&info));
    EXPECT_EQ(info.size, kTestDataSize);
    EXPECT_FALSE(info.is_directory);
    EXPECT_FALSE(info.is_symbolic_link);

    // ext2/ext3 and HPS/HPS+ seem to have a timestamp granularity of 1s.
#if defined(OS_POSIX)
    EXPECT_EQ(turbo::Time::to_timeval(info.last_accessed).tv_sec,
              turbo::Time::to_timeval(new_last_accessed).tv_sec);
    EXPECT_EQ(turbo::Time::to_timeval(info.last_modified).tv_sec,
              turbo::Time::to_timeval(new_last_modified).tv_sec);
#else
    EXPECT_EQ(info.last_accessed.ToInternalValue(),
              new_last_accessed.ToInternalValue());
    EXPECT_EQ(info.last_modified.ToInternalValue(),
              new_last_modified.ToInternalValue());
#endif

    EXPECT_EQ(info.creation_time,
              creation_time);
}

TEST(FileTest, ReadAtCurrentPosition) {
    turbo::ScopedTempDir temp_dir;
    ASSERT_TRUE(temp_dir.create_unique_temp_dir().ok());
    auto file_path = temp_dir.path() / ("read_at_current_position");
    File file(file_path,
              turbo::File::FLAG_CREATE | turbo::File::FLAG_READ |
              turbo::File::FLAG_WRITE);
    EXPECT_TRUE(file.is_valid());

    const char kData[] = "test";
    const int kDataSize = sizeof(kData) - 1;
    EXPECT_EQ(kDataSize, file.write(0, kData, kDataSize));

    EXPECT_EQ(0, file.seek(turbo::File::FROM_BEGIN, 0));

    char buffer[kDataSize];
    int first_chunk_size = kDataSize / 2;
    EXPECT_EQ(first_chunk_size, file.read_at_current_pos(buffer, first_chunk_size));
    EXPECT_EQ(kDataSize - first_chunk_size,
              file.read_at_current_pos(buffer + first_chunk_size,
                                       kDataSize - first_chunk_size));
    EXPECT_EQ(std::string(buffer, buffer + kDataSize), std::string(kData));
}

TEST(FileTest, WriteAtCurrentPosition) {
    turbo::ScopedTempDir temp_dir;
    ASSERT_TRUE(temp_dir.create_unique_temp_dir().ok());
    auto file_path = temp_dir.path() / ("write_at_current_position");
    File file(file_path,
              turbo::File::FLAG_CREATE | turbo::File::FLAG_READ |
              turbo::File::FLAG_WRITE);
    EXPECT_TRUE(file.is_valid());

    const char kData[] = "test";
    const int kDataSize = sizeof(kData) - 1;

    int first_chunk_size = kDataSize / 2;
    EXPECT_EQ(first_chunk_size, file.write_at_current_pos(kData, first_chunk_size));
    EXPECT_EQ(kDataSize - first_chunk_size,
              file.write_at_current_pos(kData + first_chunk_size,
                                        kDataSize - first_chunk_size));

    char buffer[kDataSize];
    EXPECT_EQ(kDataSize, file.read(0, buffer, kDataSize));
    EXPECT_EQ(std::string(buffer, buffer + kDataSize), std::string(kData));
}

TEST(FileTest, Seek) {
    turbo::ScopedTempDir temp_dir;
    ASSERT_TRUE(temp_dir.create_unique_temp_dir().ok());
    auto file_path = temp_dir.path() / ("seek_file");
    File file(file_path,
              turbo::File::FLAG_CREATE | turbo::File::FLAG_READ |
              turbo::File::FLAG_WRITE);
    ASSERT_TRUE(file.is_valid());

    const int64_t kOffset = 10;
    EXPECT_EQ(kOffset, file.seek(turbo::File::FROM_BEGIN, kOffset));
    EXPECT_EQ(2 * kOffset, file.seek(turbo::File::FROM_CURRENT, kOffset));
    EXPECT_EQ(kOffset, file.seek(turbo::File::FROM_CURRENT, -kOffset));
    EXPECT_TRUE(file.set_length(kOffset * 2));
    EXPECT_EQ(kOffset, file.seek(turbo::File::FROM_END, -kOffset));
}

#if defined(OS_WIN)
TEST(FileTest, GetInfoForDirectory) {
  turbo::ScopedTempDir temp_dir;
  ASSERT_TRUE(temp_dir.create_unique_temp_dir());
  FilePath empty_dir = temp_dir.path().Append(FILE_PATH_LITERAL("gpfi_test"));
  ASSERT_TRUE(CreateDirectory(empty_dir));

  turbo::File dir(
      ::CreateFile(empty_dir.value().c_str(),
                   FILE_ALL_ACCESS,
                   FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                   nullptr,
                   OPEN_EXISTING,
                   FILE_FLAG_BACKUP_SEMANTICS,  // Needed to open a directory.
                   nullptr));
  ASSERT_TRUE(dir.is_valid());

  turbo::File::Info info;
  EXPECT_TRUE(dir.get_info(&info));
  EXPECT_TRUE(info.is_directory);
  EXPECT_FALSE(info.is_symbolic_link);
  EXPECT_EQ(0, info.size);
}
#endif  // defined(OS_WIN)
