#ifndef TONKHO_H
#define TONKHO_H

#include "LoHang.h"
#include <windows.h>
#include <sql.h>
#include <sqlext.h>
#include <string>


class TonKho : public LoHang {
private:
    void LuuChiTietTonKho(SQLHDBC hDbc);
    void DongBoTongTonKho(SQLHDBC hDbc);
public:
    // Constructor
    TonKho();

    void NhapKhoVaLuu(SQLHDBC hDbc);
    void CapNhatSoLuongChiTiet(SQLHDBC hDbc,std::string maSanPham, float soLuongMoi);
    void CapNhatSoLuongTon(SQLHDBC hDbc, float soLuongMoi);

    void printInfo() const;

    bool LayThongTinTuDB(SQLHDBC hDbc, std::string maLoHangTraCuu);
};

#endif