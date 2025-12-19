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

// This header needs to be included multiple times.

#ifdef _WIN32

#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif

// The Windows API defines macros from *File resolving to either
// *FileA or *FileW.  Need to undo them.
#ifdef CopyFile
#undef CopyFile
#endif
#ifdef CreateFile
#undef CreateFile
#endif
#ifdef DeleteFile
#undef DeleteFile
#endif

// Other annoying Windows macro definitions...
#ifdef IN
#undef IN
#endif
#ifdef OUT
#undef OUT
#endif

// Note that we can't undefine OPTIONAL, because it can be used in other
// Windows headers...

#endif  // _WIN32
