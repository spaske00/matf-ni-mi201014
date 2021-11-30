//
// Created by spaske on 29.11.21..
//

#ifndef MARKO_SPASIC_NI2020_NI_LOG_H
#define MARKO_SPASIC_NI2020_NI_LOG_H

template<typename ...Args>
void log(Args&& ...args) {
#ifdef DEBUG
    println(fast_io::dbg(), std::forward<Args>(args)...);
#endif
}

#endif //MARKO_SPASIC_NI2020_NI_LOG_H
