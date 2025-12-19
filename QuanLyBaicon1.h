#ifndef QUANLYBAICON1_H
#define QUANLYBAICON1_H
#include <windows.h>
#include <sql.h>
#include <sqlext.h>

void TaiDuLieuTuDB(SQLHDBC hDbc);
void ChucNang_PhanTichABC(SQLHDBC hDbc);
void ChucNang_PhanTichTonKho(SQLHDBC hDbc);
void ChucNang_DinhGiaDong(SQLHDBC hDbc);
void ChucNang_DuBao(SQLHDBC hDbc);

#endif