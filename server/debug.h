/* debug.h - Functions for debugging and error reporting.
 * Copyright (C) 2016 Quytelda Kahja
 *
 * This file is part of EXPd.
 *
 * EXPd is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * EXPd is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with EXPd.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __DEBUG_H
#define __DEBUG_H

#define DEBUG 1

/* print_errno() - output an error message to stderr
 * Convenience macro to write error messages to stderr,
 * which are signed with an errno value.
 * Punctuation is added.
 *
 * int peer_fd = accept(fd, NULL, NULL);
 * if(peer_fd > 0) print_errno("accept() failed");
 * ==> accept() failed (errno 35).\n
 */
#define print_errno(message) fprintf(stderr, "%s (errno %d).\n", message, errno)

/* return_with_errno() - output an error message, then return
 * return_value_with_errno() - same as return_with_errno, but returns a value
 * continue_with_errno() - same as return_with_errno, but continues a loop
 * leave_with_errno() - same as return_with_errno, but jumps to the label 'leave'
 *
 * Calls print_errno() to output an error message, then returns from the caller.
 *
 * int peer_fd = accept(fd, NULL, NULL);
 * if(peer_fd > 0) return_with_errno("accept() failed");
 */
#define return_with_errno(message)              do { print_errno(message); return;       } while(0)
#define return_value_with_errno(message, value) do { print_errno(message); return value; } while(0)
#define leave_with_errno(message)               do { print_errno(message); goto leave;   } while(0)
#define continue_with_errno(message)            do { print_errno(message); continue;     } while(0)

#endif /* __DEBUG_H */
