#ifndef REF_COUNTED_H_INCLUDED
#define REF_COUNTED_H_INCLUDED

#include <type_traits>

namespace fmpire
{

class RefCounted
{
public:
	RefCounted() :
		ref_count(0)
	{
	}

	RefCounted(const RefCounted& other) :
		ref_count(0)
	{
	}

	virtual ~RefCounted() {}

	void add_ref() const { ++ref_count; }

	void release() const
	{
		--ref_count;

		if (ref_count == 0)
		{
			delete this;
		}
	}

private:
	mutable int ref_count;
};

template<class T> class Ref
{
public:
	Ref(T* p = nullptr) :
		ptr(p)
	{
		add_ref();
	}

	Ref(const Ref& other) :
		ptr(other.ptr)
	{
		add_ref();
	}

	Ref(Ref<T>&& other) :
		ptr(other.ptr)
	{
		other.ptr = nullptr;
	}

	~Ref() { release(); }

	Ref<T>& operator=(T* const obj)
	{
		if (obj != ptr)
		{
			release();
			ptr = obj;
			add_ref();
		}
		return *this;
	}

	Ref<T>& operator=(const Ref<T>& other)
	{
		if (ptr != other.ptr)
		{
			release();
			ptr = other.ptr;
			add_ref();
		}
		return *this;
	}

	Ref<T>& operator=(Ref<T>&& other)
	{
		if (ptr != other.ptr)
		{
			release();
			ptr = other.ptr;
			other.ptr = nullptr;
			add_ref();
		}
		return *this;
	}

	operator T*() const { return ptr; }

	T* operator->() const { return ptr; }

	T& operator*() const { return *ptr; }

	bool operator==(const T* other) const { return ptr == other; }

	bool operator==(const Ref<T>& other) const { return ptr == other.ptr; }

	bool operator!=(const T* other) const { return ptr != other; }

	bool operator!=(const Ref<T>& other) const { return ptr != other.ptr; }

private:
	void add_ref()
	{
		if (ptr)
		{
			ptr->add_ref();
		}
	}

	void release()
	{
		if (ptr)
		{
			ptr->release();
		}
	}

	T* ptr;
};

template<
	class T,
	class U,
	std::enable_if<std::is_base_of<U, T>::value || std::is_base_of<T, U>::value,
				   bool>::type = true>
T* static_ref_cast(const Ref<U>& ref)
{
	return static_cast<T*>((U*) ref);
}


} // namespace fmpire

#endif // REF_COUNTED_H_INCLUDED
