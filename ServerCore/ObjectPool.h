#pragma once
#include "pch.h"

template<typename T>
class ObjectPool
{
public:
	using ObjectPtr = std::shared_ptr<T>;

public:
	ObjectPool(int initCnt)
	{
		for(int i=0; i<initCnt; ++i)
		{
			pool.push_back(new T());
		}
	}

	~ObjectPool()
	{
		for(T* obj : pool)
		{
			delete obj;
		}
		pool.clear();
	}

	ObjectPtr Acquire()
	{
		std::lock_guard<std::mutex> lock(mtx);

		T* obj = nullptr;

		if (pool.empty())
			obj = new T();
		else
		{
			obj = pool.back();
			pool.pop_back();
		}

		return ObjectPtr(obj, [this](T* obj)
		{
			obj->Reset();			// 상태 초기화
			this->Release(obj);		// pool에 반환
		});
	}

private:
	void Release(T* obj)
	{
		std::lock_guard<std::mutex> lock(mtx);
		pool.push_back(obj);
	}


private:
	std::vector<T*>		pool;
	std::mutex			mtx;
};