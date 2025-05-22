/*
 * libtradier - Tradier API C++ Library v0.1.0
 *
 * Author: Benjamin Cance (kc8bws@kc8bws.com)
 * Date: 2025-05-22
 *
 * This software is provided free of charge under the MIT License.
 * By using it, you agree to absolve the author of all liability.
 * See LICENSE file for full terms and conditions.
 */

#pragma once

#define LIBTRADIER_VERSION_MAJOR 0
#define LIBTRADIER_VERSION_MINOR 1
#define LIBTRADIER_VERSION_PATCH 0

#define LIBTRADIER_VERSION_STRING "0.1.0"

#define LIBTRADIER_VERSION_HEX ((LIBTRADIER_VERSION_MAJOR << 16) | \
                              (LIBTRADIER_VERSION_MINOR << 8)  | \
                              (LIBTRADIER_VERSION_PATCH))

namespace tradier {
    struct Version {
        static constexpr int major = LIBTRADIER_VERSION_MAJOR;
        static constexpr int minor = LIBTRADIER_VERSION_MINOR;
        static constexpr int patch = LIBTRADIER_VERSION_PATCH;
        static constexpr const char* string = LIBTRADIER_VERSION_STRING;
        static constexpr unsigned int hex = LIBTRADIER_VERSION_HEX;
    };
}