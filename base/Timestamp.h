//
// Created by wxl on 2020/10/12.
//

#ifndef MAYA_TIMESTAMP_H
#define MAYA_TIMESTAMP_H

#include "copyable.h"
#include <stdint.h>
#include <algorithm>
#include <string>
#include "Types.h"
namespace maya {
    class Timestamp : public maya::copyable {
    public:
        Timestamp() : microSecondsSinceEpoch_(0) {
        }

        explicit Timestamp(int64_t microSecondsSinceEpoch) :
                microSecondsSinceEpoch_(microSecondsSinceEpoch) {
        }

        Timestamp &operator+=(Timestamp lhs) {
            this->microSecondsSinceEpoch_ += lhs.microSecondsSinceEpoch_;
            return *this;
        }

        Timestamp &operator+=(int64_t lhs) {
            this->microSecondsSinceEpoch_ += lhs;
            return *this;
        }

        Timestamp &operator-=(Timestamp lhs) {
            this->microSecondsSinceEpoch_ -= lhs.microSecondsSinceEpoch_;
            return *this;
        }

        Timestamp &operator-=(int64_t lhs) {
            this->microSecondsSinceEpoch_ -= lhs;
            return *this;
        }


        void swap(Timestamp &that) {
            std::swap(this->microSecondsSinceEpoch_, that.microSecondsSinceEpoch_);
        }

        string toString() const;//将微妙转换成秒后格式化成字符串
        string toFormattedString(bool showMicroseconds = true) const;//将微妙转换成年月日时分秒输出， bool变量控制是否输出微妙

        bool valid() { return microSecondsSinceEpoch_ > 0; }

        int64_t microSecondsSinceEpoch() { return microSecondsSinceEpoch_; };

        time_t SecondsSinceEpoch() { return microSecondsSinceEpoch_ / kMicroSecondsPerSecond; }

        static Timestamp now();

        static Timestamp invalid() {
            return Timestamp();
        }

        static Timestamp forUnixTime(time_t t) {
            return forUnixTime(t, 0);
        }

        static Timestamp forUnixTime(time_t t, int microseconds) {
            return Timestamp(static_cast<int64_t>(t) * kMicroSecondsPerSecond + microseconds);
        }

        static const int kMicroSecondsPerSecond = 1000 * 1000;
    private:
        int64_t microSecondsSinceEpoch_;
    };

    inline bool operator<(Timestamp lhs, Timestamp rhs) {
        return lhs.microSecondsSinceEpoch() < rhs.microSecondsSinceEpoch();
    }

    inline bool operator>(Timestamp lhs, Timestamp rhs) {
        return rhs < lhs;
    }

    inline bool operator<=(Timestamp lhs, Timestamp rhs) {
        return !(lhs > rhs);
    }

    inline bool operator>=(Timestamp lhs, Timestamp rhs) {
        return !(lhs < rhs);
    }

    inline bool operator==(Timestamp lhs, Timestamp rhs) {
        return lhs.microSecondsSinceEpoch() == rhs.microSecondsSinceEpoch();
    }

    inline bool operator!=(Timestamp lhs, Timestamp rhs) {
        return !(lhs == rhs);
    }

    inline double timeDifference(Timestamp high, Timestamp low) {
        int64_t diff = high.microSecondsSinceEpoch() - low.microSecondsSinceEpoch();
        return static_cast<double>(diff) / Timestamp::kMicroSecondsPerSecond;
    }

    inline Timestamp addTime(Timestamp timestamp, double seconds) {
        int64_t delta = static_cast<int64_t>(seconds * Timestamp::kMicroSecondsPerSecond);
        return Timestamp(timestamp.microSecondsSinceEpoch() + delta);
    }
}//namespace maya

#endif //MAYA_TIMESTAMP_H
