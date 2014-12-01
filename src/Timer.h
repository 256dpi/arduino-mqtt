class Timer {
public:
    Timer();
    Timer(int ms);
    bool expired();
    void countdown_ms(unsigned long ms);
    void countdown(int seconds);
    int left_ms();
private:
    unsigned long interval_end_ms; 
};
