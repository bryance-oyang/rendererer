/*
 * Copyright (c) 2023 Bryance Oyang
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef MULTIARRAY_H
#define MULTIARRAY_H

#define MULTIARRAY_MAXDIM 4

#include <cstring>
#include <utility>

template<typename T> class MultiArray {
public:
	int rank;
	int n[MULTIARRAY_MAXDIM];
	int len;
	T *data = nullptr;

	void alloc()
	{
		data = new T[len];
	}
	void free()
	{
		if (data) {
			delete[] data;
			data = nullptr;
		}
	}

	MultiArray()
	: rank{0},
	n{0, 1, 1, 1},
	len{0} {}

	MultiArray(int n0)
	: rank{1},
	n{n0, 1, 1, 1},
	len{n0} { alloc(); }

	MultiArray(int n0, int n1)
	: rank{2},
	n{n0, n1, 1, 1},
	len{n0*n1} { alloc(); }

	MultiArray(int n0, int n1, int n2)
	: rank{3},
	n{n0, n1, n2, 1},
	len{n0*n1*n2} { alloc(); }

	MultiArray(int n0, int n1, int n2, int n3)
	: rank{4},
	n{n0, n1, n2, n3},
	len{n0*n1*n2*n3} { alloc(); }

	~MultiArray()
	{
		free();
	}

	friend void swap(MultiArray &first, MultiArray &second)
	{
		using std::swap;
		swap(first.len, second.len);
		swap(first.rank, second.rank);
		swap(first.n, second.n);
		swap(first.data, second.data);
	}

	MultiArray(const MultiArray &other)
	{
		len = other.len;
		rank = other.rank;
		memcpy(n, other.n, MULTIARRAY_MAXDIM * sizeof(n[0]));

		free();
		alloc();
		memcpy(data, other.data, len * sizeof(T));
	}

	MultiArray(const MultiArray &&other)
	{
		using std::swap;
		swap(*this, other);
	}

	MultiArray &operator=(MultiArray other)
	{
		using std::swap;
		swap(*this, other);
		return *this;
	}

	MultiArray &operator+=(const MultiArray &other)
	{
		for (int i = 0; i < len; i++) {
			data[i] += other.data[i];
		}
		return *this;
	}

	T &operator()(int i0)
	{
		return data[i0];
	}
	T &operator()(int i0, int i1)
	{
		return data[i0*n[1] + i1];
	}
	T &operator()(int i0, int i1, int i2)
	{
		return data[(i0*n[1] + i1)*n[2] + i2];
	}
	T &operator()(int i0, int i1, int i2, int i3)
	{
		return data[((i0*n[1] + i1)*n[2] + i2)*n[3] + i3];
	}
};

#endif /* MULTIARRAY_H */
