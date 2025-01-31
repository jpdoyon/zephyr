/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2022 Nordic semiconductor ASA */

#include <zephyr/zephyr.h>
#include <ztest.h>
#include <errno.h>
#include <zephyr/settings/settings.h>
#include <zephyr/fs/nvs.h>

void test_setting_storage_get(void)
{
	int rc;
	void *storage;
	uint16_t data = 0x5a5a;
	ssize_t nvs_rc;

	rc = settings_storage_get(&storage);
	zassert_equal(0, rc, "Can't fetch storage reference (err=%d)", rc);

	zassert_not_null(storage, "Null reference.");

	nvs_rc = nvs_write((struct nvs_fs *)storage, 26, &data, sizeof(data));

	zassert_true(nvs_rc >= 0, "Can't read nvs record (err=%d).", rc);
}
