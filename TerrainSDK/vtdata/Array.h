//
// Array.h
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef ARRAYH
#define ARRAYH

template <class E> class Array
{
public:
	Array(int size = 0);
	Array(const Array<E>&);
	virtual ~Array();

//	Accessors
	int		GetSize() const;
	int		GetMaxSize() const;
	bool	SetSize(int);
	bool	SetMaxSize(int);
	int		GetElemSize() const;
	E*		GetData() const;
	bool	IsEmpty() const;
	E&		GetAt(int i) const;
	bool	SetAt(int, E);

//	Other operations
	Array<E>& operator=(const Array<E>&);
	E&			operator[](int i);
	const E&	operator[](int i) const;
	void		Empty();
	bool		RemoveAt(int i, int n = 1);
	int		Append(const E&);
	int		Append(const Array<E>&);
	int		Find(const E&) const;

protected:
//	Internal functions
	virtual bool	Grow(int);
	virtual void	ConstructItems(int first, int last);
	virtual	void	DestructItems(int first, int last);

//	Data members
	int		m_Size;		// number of elements added so far
	int		m_MaxSize;	// maximum number of elements we have room for
	E*		m_Data;		// data area for array
};

#endif
