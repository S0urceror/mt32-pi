//
// ringbuffer.h
//
// mt32-pi - A bare-metal Roland MT-32 emulator for Raspberry Pi
// Copyright (C) 2020  Dale Whinham <daleyo@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#ifndef _ringbuffer_h
#define _ringbuffer_h

#include <circle/spinlock.h>
#include <circle/types.h>

#include "utility.h"

template <class T, size_t N>
class CRingBuffer
{
public:
	CRingBuffer()
		: m_Lock(IRQ_LEVEL),
		  m_nInPtr(0),
		  m_nOutPtr(0),
		  m_Data{0}
	{
	}

	size_t Enqueue(const T* pItems, size_t nCount)
	{
		size_t nEnqueued = 0;
		m_Lock.Acquire();

		for (size_t i = 0; i < nCount; ++i)
		{
			if (((m_nInPtr + 1) & BufferMask) != m_nOutPtr)
			{
				m_Data[m_nInPtr++] = pItems[i];
				m_nInPtr &= BufferMask;
				++nEnqueued;
			}
		}

		m_Lock.Release();
		return nEnqueued;
	}

	size_t Dequeue(T* pOutBuffer, size_t nMaxCount)
	{
		size_t nDequeued = 0;
		m_Lock.Acquire();

		while (m_nInPtr != m_nOutPtr)
		{
			pOutBuffer[nDequeued++] = m_Data[m_nOutPtr++];
			m_nOutPtr &= BufferMask;
		}

		m_Lock.Release();
		return nDequeued;
	}

private:
	static_assert(Utility::IsPowerOfTwo(N), "Ring buffer size must be a power of 2");

	static constexpr size_t BufferMask = N - 1;

	CSpinLock m_Lock;
	volatile size_t m_nInPtr;
	volatile size_t m_nOutPtr;
	T m_Data[N];
};

#endif
