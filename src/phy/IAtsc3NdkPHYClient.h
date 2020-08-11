#ifndef LIBATSC3_IATSC3NDKPHYCLIENT_H
#define LIBATSC3_IATSC3NDKPHYCLIENT_H

class IAtsc3NdkPHYClient {
public:
    virtual int Init() = 0;
    virtual int Open(int fd, int bus, int addr) = 0;
    virtual int Tune(int freqKhz, int plp) = 0;
    virtual int Stop()  = 0;
    virtual int Close()  = 0;

    virtual ~IAtsc3NdkPHYClient() {};

    /* jjustman-2020-08-10
     * additional methods to impl?
     *
     *   int TuneMultiplePLP(int freqKhz, vector<int> plpIds);
    int ListenPLP1(int plp1); //by default, we will always listen to PLP0, append additional PLP for listening
     */
};


#endif //LIBATSC3_IATSC3NDKPHYCLIENT_H
