#ifndef SIGNAL_TRACKER_H
#define SIGNAL_TRACKER_H
// https://godbolt.org/z/jE7cv45j4

#include <atomic>
#include <csignal>
#include <optional>

template <int SignalType> class SignalTracker {
    static std::atomic<bool> signalThrown;
    static void (*previous_handler)(
        int); // c-style syntax for a pointer to a function
    static void handler(int i) {
        signalThrown = true;
        if (previous_handler)
            previous_handler(i);
    }
    SignalTracker() {
        if (previous_handler)
            return; // should never happen, but who knows
        auto prev = std::signal(SignalType, &handler);
        if (prev != SIG_ERR)
            previous_handler = prev;
    }

  public:
    static bool checkAndClear() {
        static SignalTracker s;
        return s.signalThrown.exchange(false); // Exchange value with the atomic
    }
};

// Static values for a class need to be defined outside the class
template <int SignalType>
std::atomic<bool> SignalTracker<SignalType>::signalThrown = false;

template <int SignalType>
void (*SignalTracker<SignalType>::previous_handler)(int) = nullptr;

#endif // SIGNAL_TRACKER_H
