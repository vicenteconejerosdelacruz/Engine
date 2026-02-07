#pragma once
#include <functional>

template<typename K, typename V, typename C = std::less<K>>
struct RefTracker
{
	std::map<K, V, C> instances;
	std::map<K, unsigned int> instancesRefCount;

	V& AddRef(K key, std::function<V()> newRefCallback)
	{
		if (instances.contains(key))
		{
			V& instanceRef = instances.at(key);
			instancesRefCount.find(key)->second++;
			return instanceRef;
		}
		else
		{
			V instance = newRefCallback();
			instances.insert_or_assign(key, std::move(instance));
			instancesRefCount.insert_or_assign(key, 1U);
			V& instanceRef = instances.at(key);
			return instanceRef;
		}
	}

	void IncrementRefCount(K key, unsigned int d)
	{
		instancesRefCount.find(key)->second += d;
	}

	void RemoveRef(K key)
	{
		assert(instancesRefCount.contains(key));
		instancesRefCount.at(key)--;
		if (instancesRefCount.at(key) == 0U)
		{
			instancesRefCount.erase(key);
			instances.erase(key);
		}
	}

	bool Has(K k)
	{
		return instances.contains(k);
	}

	size_t Size()
	{
		return instances.size();
	}


	V& FindValue(K key)
	{
		return instances.at(key);
	}

	void Clear()
	{
		instances.clear();
		instancesRefCount.clear();
	}
};