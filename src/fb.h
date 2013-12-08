class Facebook {
public:
    Facebook();
    ~Facebook();
    void send_packet( const char*, int );
protected:
    void print_hex_ascii_line( const char *payload, int len, int offset );
    void print_payload( const char*, int );
};
