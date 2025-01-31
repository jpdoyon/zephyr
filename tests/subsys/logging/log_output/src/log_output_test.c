/*
 * Copyright (c) 2018 Nordic Semiconductor
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Test log message
 */

#include <zephyr/logging/log.h>
#include <zephyr/logging/log_output.h>

#include <tc_util.h>
#include <stdbool.h>
#include <zephyr/zephyr.h>
#include <ztest.h>

#define LOG_MODULE_NAME test
LOG_MODULE_REGISTER(LOG_MODULE_NAME);

#define SNAME "src"
#define DNAME "domain"
#define TEST_STR "test"

static uint8_t mock_buffer[512];
static uint8_t log_output_buf[4];
static uint32_t mock_len;

static void reset_mock_buffer(void)
{
	mock_len = 0U;
	memset(mock_buffer, 0, sizeof(mock_buffer));
}

static int mock_output_func(uint8_t *buf, size_t size, void *ctx)
{
	memcpy(&mock_buffer[mock_len], buf, size);
	mock_len += size;

	return size;
}

LOG_OUTPUT_DEFINE(log_output, mock_output_func,
		  log_output_buf, sizeof(log_output_buf));

ZTEST(test_log_output, test_no_flags)
{
	char package[256];
	static const char *exp_str = SNAME ": " TEST_STR "\r\n";
	int err;

	err = cbprintf_package(package, sizeof(package), 0, TEST_STR);
	zassert_true(err > 0, NULL);

	log_output_process(&log_output, 0, NULL, SNAME, LOG_LEVEL_INF, package, NULL, 0, 0);

	mock_buffer[mock_len] = '\0';
	zassert_equal(strcmp(exp_str, mock_buffer), 0, NULL);
}

ZTEST(test_log_output, test_raw)
{
	char package[256];
	static const char *exp_str = TEST_STR;
	int err;

	err = cbprintf_package(package, sizeof(package), 0, TEST_STR);
	zassert_true(err > 0, NULL);

	log_output_process(&log_output, 0, NULL, SNAME, LOG_LEVEL_INTERNAL_RAW_STRING,
			   package, NULL, 0, 0);

	mock_buffer[mock_len] = '\0';
	zassert_equal(strcmp(exp_str, mock_buffer), 0, NULL);
}

ZTEST(test_log_output, test_no_flags_dname)
{
	char package[256];
	static const char *exp_str = DNAME "/" SNAME ": " TEST_STR "\r\n";
	int err;

	err = cbprintf_package(package, sizeof(package), 0, TEST_STR);
	zassert_true(err > 0, NULL);

	log_output_process(&log_output, 0, DNAME, SNAME, LOG_LEVEL_INF, package, NULL, 0, 0);

	mock_buffer[mock_len] = '\0';
	zassert_equal(strcmp(exp_str, mock_buffer), 0, NULL);
}

ZTEST(test_log_output, test_level_flag)
{
	char package[256];
	static const char *exp_str = "<inf> " DNAME "/" SNAME ": " TEST_STR "\r\n";
	uint32_t flags = LOG_OUTPUT_FLAG_LEVEL;
	int err;

	err = cbprintf_package(package, sizeof(package), 0, TEST_STR);
	zassert_true(err > 0, NULL);

	log_output_process(&log_output, 0, DNAME, SNAME, LOG_LEVEL_INF,
			   package, NULL, 0, flags);

	mock_buffer[mock_len] = '\0';
	zassert_equal(strcmp(exp_str, mock_buffer), 0, NULL);
}

ZTEST(test_log_output, test_ts_flag)
{
	char package[256];
	static const char *exp_str = IS_ENABLED(CONFIG_LOG_TIMESTAMP_64BIT) ?
		"[0000000000000000] " DNAME "/" SNAME ": " TEST_STR "\r\n" :
		"[00000000] " DNAME "/" SNAME ": " TEST_STR "\r\n";
	uint32_t flags = LOG_OUTPUT_FLAG_TIMESTAMP;
	int err;

	err = cbprintf_package(package, sizeof(package), 0, TEST_STR);
	zassert_true(err > 0, NULL);

	log_output_process(&log_output, 0, DNAME, SNAME, LOG_LEVEL_INF,
			   package, NULL, 0, flags);

	mock_buffer[mock_len] = '\0';
	zassert_equal(strcmp(exp_str, mock_buffer), 0, NULL);
}

ZTEST(test_log_output, test_format_ts)
{
	char package[256];
	static const char *exp_str =
		"[00:00:01.000,000] " DNAME "/" SNAME ": " TEST_STR "\r\n";
	uint32_t flags = LOG_OUTPUT_FLAG_TIMESTAMP | LOG_OUTPUT_FLAG_FORMAT_TIMESTAMP;
	int err;

	log_output_timestamp_freq_set(1000000);

	err = cbprintf_package(package, sizeof(package), 0, TEST_STR);
	zassert_true(err > 0, NULL);

	log_output_process(&log_output, 1000000, DNAME, SNAME, LOG_LEVEL_INF,
			   package, NULL, 0, flags);

	mock_buffer[mock_len] = '\0';
	printk("%s", mock_buffer);
	zassert_equal(strcmp(exp_str, mock_buffer), 0, NULL);
}

ZTEST(test_log_output, test_ts_to_us)
{
	log_output_timestamp_freq_set(1000000);

	zassert_equal(log_output_timestamp_to_us(1000), 1000, NULL);

	log_output_timestamp_freq_set(32768);

	zassert_equal(log_output_timestamp_to_us(10), 305, NULL);
}

ZTEST(test_log_output, test_levels)
{
	char package[256];
	static const char *const exp_strs[] = {
		"<err> " SNAME ": " TEST_STR "\r\n",
		"<wrn> " SNAME ": " TEST_STR "\r\n",
		"<inf> " SNAME ": " TEST_STR "\r\n",
		"<dbg> " SNAME ": " TEST_STR "\r\n"
	};
	uint8_t levels[] = {LOG_LEVEL_ERR, LOG_LEVEL_WRN, LOG_LEVEL_INF, LOG_LEVEL_DBG};
	uint32_t flags = LOG_OUTPUT_FLAG_LEVEL;
	int err;

	err = cbprintf_package(package, sizeof(package), 0, TEST_STR);
	zassert_true(err > 0, NULL);

	for (int i = 0; i < ARRAY_SIZE(exp_strs); i++) {
		reset_mock_buffer();

		log_output_process(&log_output, 0, NULL, SNAME, levels[i],
				   package, NULL, 0, flags);

		mock_buffer[mock_len] = '\0';
		zassert_equal(strcmp(exp_strs[i], mock_buffer), 0, NULL);
	}
}

ZTEST(test_log_output, test_colors)
{
#define LOG_COLOR_CODE_DEFAULT "\x1B[0m"
#define LOG_COLOR_CODE_RED     "\x1B[1;31m"
#define LOG_COLOR_CODE_GREEN   "\x1B[1;32m"
#define LOG_COLOR_CODE_YELLOW  "\x1B[1;33m"

	char package[256];
	static const char *const exp_strs[] = {
		LOG_COLOR_CODE_RED "<err> " SNAME ": " TEST_STR LOG_COLOR_CODE_DEFAULT "\r\n",
		LOG_COLOR_CODE_YELLOW "<wrn> " SNAME ": " TEST_STR LOG_COLOR_CODE_DEFAULT "\r\n",
		LOG_COLOR_CODE_DEFAULT "<inf> " SNAME ": " TEST_STR LOG_COLOR_CODE_DEFAULT "\r\n",
		LOG_COLOR_CODE_DEFAULT "<dbg> " SNAME ": " TEST_STR LOG_COLOR_CODE_DEFAULT "\r\n"
	};
	uint8_t levels[] = {LOG_LEVEL_ERR, LOG_LEVEL_WRN, LOG_LEVEL_INF, LOG_LEVEL_DBG};
	uint32_t flags = LOG_OUTPUT_FLAG_LEVEL | LOG_OUTPUT_FLAG_COLORS;
	int err;

	err = cbprintf_package(package, sizeof(package), 0, TEST_STR);
	zassert_true(err > 0, NULL);

	for (int i = 0; i < ARRAY_SIZE(exp_strs); i++) {
		reset_mock_buffer();

		log_output_process(&log_output, 0, NULL, SNAME, levels[i],
				   package, NULL, 0, flags);

		mock_buffer[mock_len] = '\0';
		zassert_equal(strcmp(exp_strs[i], mock_buffer), 0, NULL);
	}
}

static void before(void *notused)
{
	reset_mock_buffer();
}

ZTEST_SUITE(test_log_output, NULL, NULL, before, NULL, NULL);
