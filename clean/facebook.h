class FacebookClient {
  public:
    FacebookClient();
    ~FacebookClient();
    bool authenticate( const char* login, const char* password );
};
