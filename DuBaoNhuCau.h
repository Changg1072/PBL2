#ifndef DUBAONHUCAU_H
#define DUBAONHUCAU_H

#include <string>
#include <vector>
#include "Date.h"
#include "ChinhSachTon.h"
#include "ChiTietYeuCau.h"
using namespace std;
struct KetQuaDuBao {
    std::string maDuBao;         
    std::string maSP;             
    Date tuNgay;             
    Date denNgay;            
    float nhuCauTrungBinh;   
    float doLechChuan;        
};
std::string sinhMaDuBao();
KetQuaDuBao DuBaoNhuCau(const std::string& maSP,
                        const vector<ChiTietYeuCau>& dsYC,
                        Date tuNgay,
                        Date denNgay);

#endif