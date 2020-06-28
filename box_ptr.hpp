//
// Created by boat on 6/23/20.
//

#ifndef ASYNCTEST2_BOX_PTR_HPP
#define ASYNCTEST2_BOX_PTR_HPP

template<typename T>
class box_ptr {
public:
	box_ptr();
	explicit box_ptr(T inner);
	~box_ptr();
	T& operator*();
	T* operator->();
	T* get();
private:
	T* inner;
};

template<typename T>
box_ptr<T>::box_ptr() {
	inner = new T;
}

template<typename T>
box_ptr<T>::~box_ptr() {
	delete inner;
}

template<typename T>
box_ptr<T>::box_ptr(T v) {
	inner = new T(v);
}

template<typename T>
T& box_ptr<T>::operator*() {
	return *inner;
}

template<typename T>
T *box_ptr<T>::operator->() {
	return inner;
}

template<typename T>
T *box_ptr<T>::get() {
	return inner;
}


#endif //ASYNCTEST2_BOX_PTR_HPP
