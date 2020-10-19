//
// 环形队列,队列可容纳元素个数为size-1
// Created by wxl on 2020/10/16.
//

#ifndef MAYA_CIRCULARLIST_H
#define MAYA_CIRCULARLIST_H

#include <string>
#include <iostream>
#include <sstream>

//异常处理
class illegalParameterValue
{
public:
    illegalParameterValue(std::string theMessage = "Illegal parameter value")
    {message = theMessage;}
    void outputMessage() {std::cout << message << std::endl;}
private:
    std::string message;
};


template <class T>
class CricularList
{
public:
    CricularList( int initialCapacity = 10);
    ~CricularList(){delete []element;}

    T& front();
    T& back();

    void pop_front();
    void push_back(const T& theElement);

     int size() const{return (back_-front_+capacity_)%capacity_;}
     int capacity() const {return capacity_;}

    bool full() const{return size()==capacity_-1;}
    bool empty() const{return front_==back_;}
private:
    T *element;//指向队列缓冲区
     int capacity_;//队列容量
     int front_;
     int back_;
};

template <class T>
CricularList<T>::CricularList( int initialCapacity)
{
    if (initialCapacity < 1)
    {
        std::ostringstream s;
        s << "Initial capacity = " << initialCapacity << " Must be > 0";
        throw illegalParameterValue(s.str());
    }
    capacity_=initialCapacity;
    element = new T[initialCapacity];
    front_=back_=0;
}

template <class T>
T& CricularList<T>::front()
{
   if(this->empty())
   {
       std::ostringstream s;
       s << "queue is empty";
       throw illegalParameterValue(s.str());
   }
   else
   {
       return element[(front_+1)%capacity_];
   }
}

template <class T>
T& CricularList<T>::back()
{
    if(this->empty())
    {
        std::ostringstream s;
        s << "queue is empty";
        throw illegalParameterValue(s.str());
    }
    else
    {
        return element[back_];
    }
}

template <class T>
void CricularList<T>::pop_front()
{
    if(this->empty())
    {
        std::ostringstream s;
        s << "queue is empty";
        throw illegalParameterValue(s.str());
    }
    front_=(front_+1)%capacity_;
    element[front_].~T();
}
template <class T>
void CricularList<T>::push_back(const T& theElement)
{
    if(this->full())
    {
        std::ostringstream s;
        s << "queue is full";
        throw illegalParameterValue(s.str());
    }
    back_=(back_+1)%capacity_;
    element[back_]=theElement;
}
#endif //MAYA_CIRCULARLIST_H
