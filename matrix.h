#ifndef matrix_H
#define matrix_H

#define matrix_item(matrix, col_count, col, row) matrix[col + row * col_count]
#define matrix_by_cols_item(matrix, row_count, col, row) matrix[col * row_count + row]

template <typename T>
struct matrix
{
    matrix(): ptr{nullptr}, ColCount{0} {}
    matrix(T *ptr_, int ColCount_): ptr{ptr_}, ColCount{ColCount_} {}
    void set(T *ptr_, int ColCount_) { ptr=ptr_; ColCount=ColCount_; }

    T& item(int col, int row)		    { return ptr[col+row*ColCount]; }
    const T& item(int col, int row) const   { return ptr[col+row*ColCount]; }

    T* operator[](int row)		    { return ptr+row*ColCount; }
    const T* operator[](int row) const	    { return ptr+row*ColCount; }

public:
    T *ptr;
    int ColCount;
};


template <typename T>
struct matrix_by_cols
{
    matrix_by_cols(): ptr{nullptr}, RowCount{0} {}
    matrix_by_cols(T *ptr_, int RowCount_): ptr{ptr_}, RowCount{RowCount_} {}
    void set(T *ptr_, int RowCount_) { ptr=ptr_; RowCount=RowCount_; }

    T& item(int col, int row)		    { return ptr[col*RowCount+row]; }
    const T& item(int col, int row) const   { return ptr[col*RowCount+row]; }

    T* operator[](int col)		    { return ptr+col*RowCount; }
    const T* operator[](int col) const	    { return ptr+col*RowCount; }

public:
    T *ptr;
    int RowCount;
};

/*
int v[200];
for(int i=0; i<200; i++) v[i] = i+1;

matrix<int> m(v, 20);
for(int r=0; r<10; r++)
{
	for(int c=0; c<20; c++)
	{
		if(&matrix_item(v,20,c,r) == &m[r][c] && &m[r][c] == &m.item(c,r))
			cout << m[r][c] << " ";
		else cout << "err ";
	}
	cout << endl;
}
cout << endl;

matrix_by_cols<int> m_by_cols(v, 10);
for(int r=0; r<10; r++)
{
	for(int c=0; c<20; c++)
	{
		if(&matrix_by_cols_item(v,10,c,r) == &m_by_cols[c][r] && &m_by_cols[c][r] == &m_by_cols.item(c,r))
			cout << m_by_cols[c][r] << " ";
		else cout << "err ";
	}
	cout << endl;
}
cout << endl;
*/

#endif
