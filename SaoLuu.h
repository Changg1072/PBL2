#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <cstdio>
// Bao gồm thư viện ODBC
#include <windows.h>
#include <sql.h>
#include <sqlext.h>
#include <msclr/marshal_cppstd.h>
#include "SupermarketForm.h"

#include "DinhGiaDong.h"
using namespace std;
using namespace System::Collections::Generic;
using namespace PBL2QuanLyKho;
// Khai báo các lớp (giữ nguyên)
class SanPham;
class Kho;
class NhaCungCap;
class LoHang;
class SieuThi;
class ChiTietYeuCau;
class ChinhSachTon;
struct LichSuBanHang;
struct KetQuaDuBao;
class PhuongTien;
class TuyenVanChuyen;

// Các hàm sinh mã (giữ nguyên)
std::string trim_right(const std::string& source);
std::string sinhMaSanPham();
std::string sinhMaGia();
std::string sinhMaKho();
std::string sinhMaNhaCungCap();
std::string sinhMaLoHang();
std::string sinhMaSieuThi();// Thêm (nếu cần, dựa trên bảng)
std::string sinhMaChinhSach(); 
std::string sinhMaPhuongTien();
std::string sinhMaTuyen(); // Thêm (nếu cần)

// --- Helper Functions ---
void check_error(SQLHANDLE handle, SQLSMALLINT handle_type, SQLRETURN retcode, const std::string& msg);
std::wstring s2ws(const std::string& s);
std::string ws2s(const std::wstring& wstr);

// --- SAN PHAM ---
// Chữ ký hàm đã thay đổi để nhận SQLHDBC
void GhifileSanPham(SQLHDBC hDbc, SanPham& sp);
void DocfileSanPham(SQLHDBC hDbc, std::vector<SanPham>& spList);
void PrintfSanPham(std::vector<SanPham>& spList); // Hàm này không cần hDbc nếu nó chỉ in từ vector
void GhiDeFileSanPham(SQLHDBC hDbc, const std::vector<SanPham>& spList);

// --- KHO ---
void GhifileKho(SQLHDBC hDbc, Kho& k);
void DocfileKho(SQLHDBC hDbc, std::vector<Kho>& khoList);

// --- NHA CUNG CAP ---
void GhifileNhaCungCap(SQLHDBC hDbc, NhaCungCap& ncc);
void DocfileNhaCungCap(SQLHDBC hDbc, std::vector<NhaCungCap>& nccList);

// --- LO HANG --- (Lưu ý: Bảng này có master-detail)
void GhifileLoHang(SQLHDBC hDbc, LoHang& lo);
void DocfileLoHang(SQLHDBC hDbc, std::vector<LoHang>& loList, const std::vector<SanPham>& spList, const std::vector<Kho>& khoList, const std::vector<NhaCungCap>& nccList);

// --- SIEU THI ---
void GhifileSieuThi(SQLHDBC hDbc, SieuThi& st);
void DocfileSieuThi(SQLHDBC hDbc, std::vector<SieuThi>& stList);

// --- CHI TIET YEU CAU --- (Lưu ý: Bảng này có master-detail)
// Giả sử ChiTietYeuCau cũng chứa thông tin của YeuCau (MaSieuThi, NgayYeuCau)
void GhiFileYeuCau(SQLHDBC hDbc, const ChiTietYeuCau& ctyc, const std::string& maSieuThi); // Cần thêm MaSieuThi
void DocFileYeuCau(SQLHDBC hDbc, std::vector<ChiTietYeuCau>& dsYC, std::vector<SanPham>& dsSP);
void CapNhatTrangThaiYeuCau(SQLHDBC hDbc, std::string maYeuCau, std::string trangThaiMoi);

//--- CHINH SACH TON ----
void DocFileChinhSachTon(SQLHDBC hDbc, std::vector<ChinhSachTon>& dsCS, const std::vector<SanPham>& dsSP);
void GhiDeFileChinhSachTon(SQLHDBC hDbc,const std::vector<ChinhSachTon>& dsCS);
// Hàm này sẽ dùng logic UPDATE / INSERT (UPSERT)
void CapNhatGhiFileChinhSachTon(SQLHDBC hDbc, const ChinhSachTon& csMoi);

//--- DINH GIA DONG ---
void GhiFileDinhGia(SQLHDBC hDbc, const DinhGiaDong::KetQuaDinhGia& kq);
void DocFileDinhGia(SQLHDBC hDbc, std::vector<DinhGiaDong::KetQuaDinhGia>& dsKQ);

//--- DU BAO NHU CAU ---
void DocFileDuBaoNhuCau(SQLHDBC hDbc, std::vector<KetQuaDuBao>& dsDuBao);
void GhiFileDuBaoNhuCau(SQLHDBC hDbc, const KetQuaDuBao& kq);

//--- PHUONG TIEN ---
void GhifilePhuongTien(SQLHDBC hDbc, PhuongTien& pt);
void DocfilePhuongTien(SQLHDBC hDbc, std::vector<PhuongTien>& ptList);
//--- TUYEN VAN CHUYEN ----
void DocfileTuyenVanChuyen(SQLHDBC hDbc, std::vector<TuyenVanChuyen>& dsTVC);
void GhifileTuyenVanChuyen(SQLHDBC hDbc, TuyenVanChuyen& tvc);
//---LAY DU LIEU TU TON KHO---
float LayTongTonKhoHienTai(SQLHDBC hDbc, const std::string& maSP);
float LayTonKhoSapHetHan(SQLHDBC hDbc, const std::string& maSP, int soNgayCanhBao);
//---PHAN BO NGUON HANG---
void GhiNhatKyNguonHang(SQLHDBC hDbc, std::string maYeuCau, std::string maKho, std::wstring loaiNguon, std::wstring ghiChu);
//---LẤY TỒN KHO TẠI 1 KHO----
float LayTonKhoTaiKho(SQLHDBC hDbc, const std::string& maKho, const std::string& maSP);