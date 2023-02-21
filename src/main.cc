/*
 * Copyright (c) 2023 Bryance Oyang
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <cstdio>
#include <unistd.h>

#include <vector>
#include <thread>
#include "macro_def.h"

using namespace std;

void test(int &x)
{
	printf("%d start\n", x);
	sleep(5);
	printf("%d end\n", x);
}

int main()
{
	vector<thread> threads;

	for (int i = 0; i < 10; i++) {
		threads.emplace_back(thread(test, ref(i)));
		sleep(1);
	}
	for (int i = 0; i < 10; i++) {
		threads[i].join();
	}
	return 0;
}
