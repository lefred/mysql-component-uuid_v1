/* Copyright (c) 2017, 2024, Oracle and/or its affiliates. All rights reserved.
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License, version 2.0,
  as published by the Free Software Foundation.
  This program is also distributed with certain software (including
  but not limited to OpenSSL) that is licensed under separate terms,
  as designated in a particular file or component or in included license
  documentation.  The authors of MySQL hereby grant you an additional
  permission to link the program and your derivative works with the
  separately licensed software that they have included with MySQL.
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License, version 2.0, for more details.
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#define LOG_COMPONENT_TAG "uuid_v1"

#define UUID_T_LENGTH (16)
#define UNIX_TS_LENGTH (6)
#define RAND_A_LENGTH (2)
#define RAND_B_LENGTH (8)

#include <mysql/components/component_implementation.h>
#include <mysql/components/services/log_builtins.h> /* LogComponentErr */
#include <mysqld_error.h>                           /* Errors */
#include <mysql/components/services/udf_metadata.h>
#include <mysql/components/services/udf_registration.h>
#include <mysql/components/services/mysql_runtime_error_service.h>

#include <list>
#include <string>

#include <openssl/rand.h>  // RAND_bytes

#include <ctime>
#include <iostream>
#include <iomanip>

#include <boost/locale/date_time.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/algorithm/string.hpp>

// Return human readable format like
// 2023-08-11 08:08:03.373
std::string get_timestamp(uint64_t milliseconds) {
    std::time_t seconds = milliseconds / 1000;
    std::tm timeinfo;
#ifdef _WIN32
    localtime_s(&timeinfo, &seconds); // Use localtime_s for Windows
#else
    localtime_r(&seconds, &timeinfo); // Use localtime_r for Linux/Unix
#endif

    std::ostringstream oss;
    oss << std::put_time(&timeinfo, "%Y-%m-%d %H:%M:%S") 
	    << '.' << std::setfill('0') << std::setw(3) << milliseconds % 1000;
    return oss.str();
}

// Return longer human readable format like
// Fri Aug 11 08:08:03 2023 CEST
std::string get_timestamp_long(uint64_t milliseconds) {
    std::time_t seconds = milliseconds / 1000;
    std::tm timeinfo;
#ifdef _WIN32
    localtime_s(&timeinfo, &seconds); // Use localtime_s for Windows
#else
    localtime_r(&seconds, &timeinfo); // Use localtime_r for Linux/Unix
#endif

    std::ostringstream oss;
    oss << std::put_time(&timeinfo, "%c %Z");

    return oss.str();
}

// We don't really convert it, we just perform some checks
int string_to_uuid(const std::string &str) {
    if (str.size() != 36) {
	mysql_error_service_emit_printf(mysql_service_mysql_runtime_error,
                ER_UDF_ERROR, 0, "uuid_to_timestamp", "Invalid UUID string length");
	return 1;
    }
    if (str[14] != '1') {
	mysql_error_service_emit_printf(mysql_service_mysql_runtime_error,
                ER_UDF_ERROR, 0, "uuid_to_timestamp", "This is not a UUID v1");
	return 1;
    }

    return 0;
}

uint64_t uuid_to_unixts(const std::string &uuid_str) {

  	static constexpr const int MS_FROM_100NS_FACTOR = 10000;
	static constexpr const uint64_t OFFSET_FROM_15_10_1582_TO_EPOCH = 122192928000000000;

  	/* store uuid parts in a vector */
  	std::vector<std::string> uuid_parts;

  	/* split uuid with '-' as delimiter */
  	boost::split(uuid_parts, uuid_str, [](char c){return c == '-';});

  	/* first part of uuid is time-low
     	   second part is time-mid
     	   third part is time high with most significant 4 bits as uuid version
	*/
  	std::string uuid_timestamp = uuid_parts[2].substr(1) + uuid_parts[1] + uuid_parts[0];

  	uint64_t timestamp = std::stoul(uuid_timestamp, nullptr, 16);

  	return (timestamp - OFFSET_FROM_15_10_1582_TO_EPOCH) / MS_FROM_100NS_FACTOR;
}

std::string uuid_to_ts(const std::string &uuid_str) {
        std::string out;
        uint64_t timestamp  = uuid_to_unixts(uuid_str); 
	time_t t = static_cast<time_t>(timestamp/1000);
	// Convert to local time
        struct tm *ltm = localtime(&t);
        // Create a buffer to hold the formatted date/time string
        char buffer[80];
	strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", ltm);
        std::ostringstream oss;
        oss << buffer << '.' << std::setfill('0') << std::setw(3) << timestamp % 1000;
	out = oss.str();

        return out;
}

std::string uuid_to_ts_long(const std::string &uuid_str) {
        std::string out;
        uint64_t timestamp  = uuid_to_unixts(uuid_str);
        time_t t = static_cast<time_t>(timestamp/1000);
        // Convert to local time
        struct tm *ltm = localtime(&t);
        // Create a buffer to hold the formatted date/time string
        char buffer[80];
	strftime(buffer, sizeof(buffer), "%c %Z", ltm);
	out = buffer;

        return out;

}


extern REQUIRES_SERVICE_PLACEHOLDER(log_builtins);
extern REQUIRES_SERVICE_PLACEHOLDER(log_builtins_string);
extern REQUIRES_SERVICE_PLACEHOLDER(udf_registration);
extern REQUIRES_SERVICE_PLACEHOLDER(mysql_udf_metadata);
extern REQUIRES_SERVICE_PLACEHOLDER(mysql_runtime_error);

extern SERVICE_TYPE(log_builtins) * log_bi;
extern SERVICE_TYPE(log_builtins_string) * log_bs;
