//
// Array.inl
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef ARRAYINL
#define ARRAYINL

#include <stdlib.h>	// for free()
#include <memory.h>	// for memcpy()
#include <assert.h>	// for assert()

/*++++
 *
 * Name: Array_Array
 *	Array<E>(int size, E* data)
 *	Array(const Array&)
 *
 * Description:
 * Creates and initializes an empty array (array with no elements).
 *
 * Args:
 *	<size>	number of elements data area should initially make
 *			room for (initial value of [MaxSize]).
 *			If zero, little initial space is allocated but the
 *			array will grow dynamically as space is needed.
 *
 * Also: ISMObj Array_SetMaxSize Array_SetSize
 *
 * Examples:
 *	Array<void*> foo;            // empty array of pointers
 *
 ----*/
template <class E> Array<E>::Array(int size)
{
	m_Size = 0;					// empty to start
	m_MaxSize = 0;			// remember the size
	m_Data = NULL;
	if (size > 0)				// make room for <size> elements
		Grow(size);
}


/*++++
 *
 * Name: Array_ConstructItems
 *	ConstructItems(int first, int last)
 *
 * Description:
 *	Called by the array implementation when new array items are created.
 *	The default implementation does <<not>> call the constructors for the
 *	array items, it just fills the data with zeros. This will not work if
 *	your array elements use virtual functions.
 *
 *	Override this function to explicitly call the constructors properly if
 *	you need this functionality. In order to call the constructors without
 *	allocating memory, you need to supply a special version of <operator new>
 *	for your array elements.
 *
 * Args:
 *	<first>	index of first element to make
 *	<last>	index of last element to make
 *
 * Example:
 *	// Overrides ConstructItems to call constructors
 *	void* MyElem::operator new(size_t s, void* p) { return p; }
 *	typedef Array<MyElem> MyArray;
 *	inline void MyArray::ConstructItems(int first, int last)
 *	{
 *		for (int i = first; i <= last; ++i)
 *		   new (GetData() + i) MyElem();
 *	}
 *
 * Also: Array_Grow Array_DestructItems
 *
 ----*/
template <class E> inline void Array<E>::ConstructItems(int start, int last)
{
	memset(m_Data + start, 0, (last - start + 1) * sizeof(E));
}

/*++++
 *
 * Name: Array_DestructItems
 *	DenstructItems(int first, int last)
 *
 * Description:
 *	Called by the array implementation when array items are deleted.
 *	The default implementation does <<not>> call the destructors for the
 *	array items. This will not work if your array elements do memory
 *	deallocation in their destructors.
 *
 *	Override this function to explicitly call the destructors properly if
 *	you need this functionality.
 *
 * Args:
 *	<first>	index of first element to destroy
 *	<last>	index of last element to destroy
 *
 * Example:
 *	// Overrides DestructItems to call constructors
 *	inline void MyArray::DestructItems(int first, int last)
 *	{
 *		for (int i = first; i <= last; ++i)
 *		   (GetData() + i)->~MyElem();
 *	}
 *
 * Also: Array_Grow Array_ConstructItems
 *
 ----*/
template <class E> inline void Array<E>::DestructItems(int start, int nitems) { }


/****
 *
 *	Destructor for array class. Calls the destructor for
 *	all remaining array elements and then frees data area
 *	(if it is not user managed)
 *
 ****/
template <class E> inline Array<E>::~Array()
{
	Empty();
	free(m_Data);
	m_Data = NULL;
	m_MaxSize = 0;
}

/*++++
 *
 * Name: Array_Grow
 * 	Grow(int growto)
 *
 * Description:
 * Enlarge the array to accomodate <growto> elements. If the array
 * can already hold this many elements, nothing is done. Otherwise,
 * the data area of the array is enlarged (reallocating if necessary)
 * so that <growto> contiguous elements can be stored.
 *
 * Args:
 *	<growto>	Number of elements the array should be able to
 *				hold after it has grown
 *
 * Returns: bool
 *	<true> if array was successfully grown, else <false>
 *
 * Also: Array_SetData Array_SetMaxSize
 *
 ----*/
template <class E> bool Array<E>::Grow(int growto)
{
	int	n = growto - m_MaxSize;
	E*		old_data;

	if (n >= 0)
	   {
		if (m_Data == NULL)					// make new data area?
		   {
//			assert(m_Size == 0);
			m_Data = (E*) malloc(sizeof(E) * growto);
			if (m_Data == NULL)				// could not enlarge?
//				GFX_ERROR(("Array_Grow: out of memory"), false);
				return false;
		   }
		else
		   {
			if (growto < 12) growto = 12;	// minimum growth
			old_data = m_Data;
			m_Data = (E*) malloc(sizeof(E) * (growto));
			memcpy(m_Data, old_data, sizeof(E) * m_Size);
			free(old_data);
			if (m_Data == NULL)
//				GFX_ERROR(("Array_Grow: out of memory"), false);
				return false;
		   }
		m_MaxSize = growto;					// remember new size
		ConstructItems(m_Size, m_MaxSize - 1);
	   }
	return true;
}

template <class E> inline E* Array<E>::GetData() const
	{ return m_Data; }

/*++++
 *
 * Name: Array_SetMaxSize
 *	bool SetMaxSize(int s)
 *	int GetMaxSize()
 *
 * Description:
 * If the array is user-managed, MaxSize establishes the
 * maximum number of elements that can be stored. If the array
 * is dynamically managed by the system, setting the maximum size
 * enlarges its data area to accomodate the required number of elements.
 *
 * Args:
 *	<s>	Current maximum size of array (number of elements
 *		its data area can hold)
 *
 * Returns: bool
 *	<true> if maximum size successfully changed, else <false>
 *
 * Also: Array_SetData Array_SetSize Array_GetElemSize Array_Grow
 *
 ----*/
template <class E> bool Array<E>::SetMaxSize(int s)
{
	if (s > m_MaxSize)				// enlarge array
		return Grow(s);				// if we can
	return true;
}

template <class E> inline int	Array<E>::GetMaxSize() const
	{ return m_MaxSize; }

template <class E> inline int	Array<E>::GetElemSize() const
	{ return sizeof(E); }

/*++++
 *
 * Name: Array_SetSize
 *	bool SetSize(int s)
 * 	int GetSize()
 *
 * Description:
 * The current array size. If the array is dynamically managed,
 * it will be enlarged to accomodate the new size. If not,
 * the array size cannot be set beyond the current maximum size.
 * When the array size is enlarged, we should call constructors
 * for the new empty elements. We don't do that yet.
 *
 * Args:
 *	<s>	Current number of elements contained in array
 *
 * Returns: bool
 *	<true> if size successfully changed, else <false>
 *
 * Also: Array_SetData Array_Grow Array_SetMaxSize
 *
 * Examples:
 *	Array<ISMCol4f> cols(256);   // room for 256 colors
 *	int ncols = cols.GetSize();     // will be zero
 *	ncols = cols.GetMaxSize();      // will be 256
 *	cols.SetSize(ncols);            // calls 256 Color4 constructors NOT
 *
 ----*/
template <class E> bool inline Array<E>::SetSize(int s)
{
	assert(s >= 0);
	if (s > m_MaxSize)
		if (!Grow(s))
			return false;
	m_Size = s;
	return true;
}

template <class E> inline int	Array<E>::GetSize() const
	{ return m_Size; }

/*++++
 *
 * Name: Array_SetAt
 *	SetAt(int i, E& p)
 *	E& GetAt(int i)
 *
 * Description:
 *	Sets the <i>th element of the array to the given value.
 *	The number of bytes copied is determined by the element size
 *	of the array specified at initialization time.
 *	If the array is not large enough, it is extended to become
 *	1 1/2 times its current size.
 *
 * Args:
 *	<i>		index of new element (0 based)
 *	<p>		pointer to the value of the new element
 *
 * Returns: int
 *	index of element added or -1 if out of memory
 *
 * Examples:
 *	Array<ISMCol4f> cols(16);        // room for 16 colors
 *	cols.SetAt(0, ISMCol4f(1,1,1));     // first color white
 *	cols.SetAt(15, ISMCol4f(1,0,0));    // last color red
 *                                      // makes Colors 1-14, too
 *	cols.SetAt(17, ISMCol4f(0,1,1));    // causes array growth
 *
 ----*/
template <class E> bool Array<E>::SetAt(int i, E p)
{
	if (i >= m_MaxSize)			// need to extend array?
	   {
		int n = m_MaxSize;
		n += (n >> 1);			// grow to 1 1/2 times current size
		if (n <= i) n = i + 1;	// unless user wants more
		if (!Grow(n))			// extend failure
			return false;
	   }
	m_Data[i] = p;
	if (i >= m_Size)			// enlarge array size if at end
		m_Size = i + 1;
	return true;
}

/*++++
 *
 * Name: Array_Operators
 *	E& GetAt(int index)
 *	E& Array[int index]
 *
 * Description:
 *	Gets the <i>th element of the array.
 *
 * Args:
 *	<index>		0 based index of element to get. Like C++ arrays,
 *				these arrays do not check the range of the index so your
 *				program will crash if you supply a number less than zero
 *				or greater than the array size as an index.
 *
 * Returns:
 *	element accessed
 *
 ----*/
template <class E> inline E& Array<E>::GetAt(int i) const
	{ return m_Data[i]; }

template <class E> inline const E& Array<E>::operator[](int i) const
	{ return m_Data[i]; }

template <class E> inline E& Array<E>::operator[](int i)
	{ return m_Data[i]; }

/*++++
 *
 * Name: Array_Find
 *	int Find(E& elem)
 *
 * Description:
 * Compares each element of the array to the input element and
 * returns the index of the first one that matches. Comparison
 * of elements is done using operator== (which must be defined
 * for your element class if you want to use Find).
 *
 * Args:
 *	<elem>	value of the element to match
 *
 * Returns: int
 *	index of matching array element or -1 if not found
 *
 * Also: Array_SetAt
 *
 * Examples:
 *	Array<int> foo;   // define integer array
 *	foo.Append(5);         // first element is 5
 *	foo.Append(6);         // second element is 5
 *	int t = foo.Find(7);   // returns -1
 *	t = foo.Find(6);       // returns 1
 *
 ----*/
template <class E> int Array<E>::Find(const E& elem) const
{
	const E* p = m_Data;

	for (int i = 0; i < m_Size; ++i)	// look for matching element
		if (*p++ == elem)
			return i;					// found it
	return -1;
}

/*++++
 *
 * Name: Array_Append
 *	int Append(E v)
 *
 * Description:
 * Appends one element onto the end of the array.
 * If the array is dynamically managed, it is enlarged
 * to accomodate another element if necessary.
 *
 * Args:
 *	<v>	 value of the new element
 *
 * Returns: void*
 *	-> element added (within array) or NULL if out of memory
 *
 * Also: Array_SetAt Array_SetSize Array_RemoveAt
 *
 * Examples:
 *	Array<float>	vals;
 *	vals.Append(1.0f);      // first element
 *	vals.Append(2.0f);      // second element
 *	vals.SetAt(5, 6.0f);    // sixth element
 *	vals.Append(7.0f);      // seventh element
 *
 ----*/
template <class E> int inline Array<E>::Append(const E& v)
{
	int	index = m_Size;

	if (!SetAt(index, v))
		return -1;
	return index;
}

/*++++
 *
 * Name: Array_RemoveAt
 *	RemoveAt(int i)
 *
 * Description:
 *	Removes the <i>th element of the array. The following
 *	elements are shuffled up to eliminate the unused space.
 *
 * Args:
 *	<i>		index of element to remove (0 based)
 *
 * Returns: bool
 *	<true> if element was successfully removed, else <false>
 *
 * Also: Array_Append Array_SetAt Array_SetSize
 *
 * Examples:
 *	Array<int16> zot(8); // room for 8 shorts
 *	zot.SetSize(8);         // now has 8 zeros
 *	zot[1] = 1;             // second element
 *	zot[2] = 2;             // third element
 *	zot.RemoveAt(0);        // remove first element
 *	zot.RemoveAt(-1);       // returns false
 *
 ----*/
template <class E> bool Array<E>::RemoveAt(int i, int n)
{
	E*	elem;
	int	shuffle;

	if ((i < 0) || (i >= m_Size))		// element out of range?
		return false;
	if (n == 0) n = 1;					// default is one element
	shuffle = m_Size - (i + n);			// number to shuffle up
	elem = m_Data + i;
	memcpy(elem, elem + n, sizeof(E) * shuffle);
	m_Size -= n;
	return true;
}

/*++++
 *
 * Name: Array_Append
 *	Append(Array& src)
 *
 * Description:
 * Concatenates the contents of the source array into the destination array.
 * The destination array is enlarged if possible. When an object array
 * is appended, the objects are multiply referenced (not duplicated).
 *
 * Args:
 *	<src>	Source array containing elements to be appended
 *
 * Returns: int
 *	index of last element successfully added or -1 on error
 *
 * Also: Array_Append Array_SetAt
 *
 * Examples:
 *	int	zap[3] = { 1, 2, 3 };
 *	Array<int> zip(3, &zap);  // user managed
 *	Array<int> zot;           // dynamic
 *	zot.Append(zip);               // adds 1 2 3
 *	zot.Append(zip);               // adds 1 2 3 again
 *
 ----*/
template <class E> int Array<E>::Append(const Array<E>& src)
{
	int	n = m_Size + src.m_Size;

	if (!Grow(n))
		return -1;
	for (int i = 0; i < src.m_Size; ++i)
		m_Data[m_Size + i] = src.m_Data[i];
	m_MaxSize = n;
	m_Size += src.m_Size;
	return n - 1;
}

/*++++
 *
 * Name: Array_Empty
 *	Empty()
 *
 * Description:
 * Removes the elements in the array but not the array data area.
 * An array is considered empty if it has no elements.
 *
 * Also: Array_SetSize Array_IsEmpty
 *
 ----*/
template <class E> void Array<E>::Empty()
{
	DestructItems(0, m_Size - 1);
	m_Size = 0;
}

/*++++
 *
 * Name: Array_IsEmpty
 *	bool IsEmpty()
 *
 * Description:
 * Determines whether an array has elements or not
 *
 * Returns: bool
 *	<true> if array contains no elements, else <false>
 *
 * Also: Array_SetSize Array_Empty
 *
 ----*/
template <class E> inline bool Array<E>::IsEmpty() const
	{ return m_Size == 0; }

#endif
