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
			++createdCnt;
		}
	}

	~ObjectPool()
	{
		std::lock_guard<std::mutex> lock(mtx);

		PLOGE << "[ObjectPool::~ObjectPool] created=" << createdCnt
			<< ", acquired=" << acquiredCnt
			<< ", released=" << releasedCnt
			<< ", pool=" << pool.size();

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
		{
			obj = new T();
			++createdCnt;
		}
		else
		{
			obj = pool.back();
			pool.pop_back();
		}

		++acquiredCnt;

		return ObjectPtr(obj, [this](T* obj)
		{
			//PLOGD << "반환 : " << obj;
			obj->Reset();			// 상태 초기화
			this->Release(obj);		// pool에 반환
		});
	}

private:
	void Release(T* obj)
	{
		std::lock_guard<std::mutex> lock(mtx);

		if (std::find(pool.begin(), pool.end(), obj) != pool.end())
		{
			PLOGE << "[ObjectPool ERROR] " << obj;
			return;
		}

		pool.push_back(obj);
		++releasedCnt;
		// PLOGI << "[ObjectPool Release] obj = " << obj << ", pool size = " << pool.size();  // new2
	}


private:
	std::vector<T*>		pool;
	std::mutex			mtx;

	std::atomic<int>	createdCnt = 0;
	std::atomic<int>	acquiredCnt = 0;
	std::atomic<int>	releasedCnt = 0;
};