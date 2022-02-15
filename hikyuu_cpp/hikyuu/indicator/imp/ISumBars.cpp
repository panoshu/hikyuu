/*
 * ISumBars.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-5
 *      Author: fasiondog
 */

#include "ISumBars.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::ISumBars)
#endif

namespace hku {

ISumBars::ISumBars() : IndicatorImp("SUMBARS", 1) {
    setParam<double>("a", 0);
}

ISumBars::~ISumBars() {}

bool ISumBars::check() {
    return true;
}

void ISumBars::_calculate(const Indicator& ind) {
    size_t total = ind.size();
    m_discard = ind.discard();
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    double a = getParam<double>("a");
    if (total == m_discard + 1) {
        if (ind[m_discard] >= a) {
            _set(0, m_discard);
        } else {
            m_discard = total;
        }
        return;
    }

    size_t start = total - 1;
    size_t pos = start;
    size_t last_pos = start;
    double sum = ind[pos];
    for (size_t i = start; i >= m_discard; i--) {
        if (i != start) {
            sum = sum - ind[i + 1];
        }

        if (i < pos) {
            sum = ind[i];
            pos = i;
        }

        if (sum < a) {
            if (pos >= 1) {
                for (size_t j = pos - 1; j >= m_discard; j--) {
                    sum += ind[j];
                    if (sum >= a) {
                        pos = j;
                        break;
                    }

                    if (j == m_discard) {
                        pos = Null<size_t>();
                        break;
                    }
                }
            } else {
                pos = Null<size_t>();
            }
        }

        if (pos != Null<size_t>()) {
            _set(i - pos, i);
        }

        if (i == m_discard || pos == Null<size_t>()) {
            last_pos = i;
            break;
        }
    }

    m_discard = pos == Null<size_t>() ? last_pos + 1 : last_pos;
}

void ISumBars::_dyn_calculate(const Indicator& ind) {
    Indicator ind_param(getIndParamImp("a"));
    HKU_CHECK(ind_param.size() == ind.size(), "ind_param->size()={}, ind.size()={}!",
              ind_param.size(), ind.size());
    m_discard = std::max(ind.discard(), ind_param.discard());
    size_t total = ind.size();
    HKU_IF_RETURN(0 == total || m_discard >= total, void());

    for (size_t i = m_discard; i < total; i++) {
        price_t a = ind_param[i];
        price_t sum = 0.0;
        price_t n = Null<price_t>();
        for (size_t j = i; j >= ind.discard(); j--) {
            sum += ind[j];
            if (sum >= a) {
                n = price_t(j - i);
                break;
            }
            if (j == ind.discard()) {
                break;
            }
        }
        _set(n, i);
    }
}

Indicator HKU_API SUMBARS(double a) {
    IndicatorImpPtr p = make_shared<ISumBars>();
    p->setParam<double>("a", a);
    return Indicator(p);
}

Indicator HKU_API SUMBARS(const IndParam& a) {
    IndicatorImpPtr p = make_shared<ISumBars>();
    p->setIndParam("a", a);
    return Indicator(p);
}

} /* namespace hku */
