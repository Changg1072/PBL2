#ifndef HETHONGQUANLY_H
#define HETHONGQUANLY_H
#include <windows.h>
#include <sql.h>
#include <sqlext.h>

void Menu_QuanLyNoiBo(SQLHDBC hDbc);
void Menu_QuanLyNhapHang(SQLHDBC hDbc);
void Menu_QuanLyVanHanh(SQLHDBC hDbc);
void Menu_QuanLyVanChuyen(SQLHDBC hDbc);

// Các hàm chức năng cũng phải nhận hDbc
void ChucNang_ToiUuVaDinhGia(SQLHDBC hDbc);
void ChucNang_ToiUuKho(SQLHDBC hDbc);
void ChucNang_ToiUuDienNang(SQLHDBC hDbc);

#endif