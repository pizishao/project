#pragma once

#define LockGuarder(mtx) std::lock_guard<std::mutex> lockGuard_##mtx (mtx);