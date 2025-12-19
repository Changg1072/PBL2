#include <iostream>
#include <string>
#include <vector>
#include "SanPham.h"
#include "SieuThi.h"
#include "Kho.h"
#include "ChiTietYeuCau.h"
using namespace std;
ChiTietYeuCau::ChiTietYeuCau(){
    MaYeuCau = "";
    dsYeuCau.clear();
    soLuongTan = 0.0f;
    giaTri =0.0f;
    HoanThanh = "C";
}
ChiTietYeuCau::ChiTietYeuCau(std::string MaYeuCau,const std::vector<SanPhamYeuCau>& dsYeuCau){
    setMaYeuCau(MaYeuCau);
    setdsYeuCau(dsYeuCau);
    this->soLuongTan = 0.0f;
    this->giaTri = 0.0f;
    this->HoanThanh = "C";
    for (const auto& yc : dsYeuCau) { 
        this->soLuongTan += yc.soLuongTan;
        this->giaTri += yc.ThanhTienRieng; 
    }
}
std::string ChiTietYeuCau::getMaYeuCau() const{
    return MaYeuCau;
}
float ChiTietYeuCau::getGiaTri() const{
    return giaTri;
}
float ChiTietYeuCau::getSoLuongTan() const{
    return soLuongTan;
}
std::vector<SanPhamYeuCau> ChiTietYeuCau::getdsYeucau() const{
    return dsYeuCau;
}
void ChiTietYeuCau::setdsYeuCau(std::vector<SanPhamYeuCau> ds){
    this->dsYeuCau = ds;
}
void ChiTietYeuCau::setGiaTri(float giaTri){
    this->giaTri = giaTri;
}
void ChiTietYeuCau::setMaYeuCau(std::string MaYeuCau){
    this->MaYeuCau = MaYeuCau;
}
void ChiTietYeuCau::setSoLuongTan(float soLuongTan){
    this->soLuongTan = soLuongTan;
}
void ChiTietYeuCau::setNgayNhap(Date NgayNhap){
    this->NgayNhap = NgayNhap;
}
Date ChiTietYeuCau::getNgayNhap() const{
    return NgayNhap;
}
void ChiTietYeuCau::setHoanThanh(std::string status) {
    this->HoanThanh = status;
}
std::string ChiTietYeuCau::getHoanThanh() const {
    return this->HoanThanh;
}