#pragma once
#include <vector>
#include "DataModels.h"
namespace PBL2QuanLyKho {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Windows::Forms;
	using namespace System::Collections::Generic;
	using namespace System::Drawing;

	public ref class RestockAlertForm : public System::Windows::Forms::Form
	{
	public:
		RestockAlertForm(List<CanhBaoItem^>^ listData) {
			InitializeComponent();
			LoadData(listData);
		}

	private:
		System::Windows::Forms::DataGridView^ dgv;
		void InitializeComponent() {
			this->Size = System::Drawing::Size(800, 500);
			this->Text = L"Cảnh báo nhập hàng";
			this->StartPosition = FormStartPosition::CenterParent;

			Label^ lbl = gcnew Label();
			lbl->Text = L"⚠️ DANH SÁCH CẦN NHẬP HÀNG (Dựa trên Chính sách tồn)";
			lbl->Dock = DockStyle::Top; lbl->Height = 50;
			lbl->Font = (gcnew System::Drawing::Font(L"Segoe UI", 12, FontStyle::Bold));
			lbl->ForeColor = Color::Red; lbl->TextAlign = ContentAlignment::MiddleCenter;

			dgv = gcnew DataGridView();
			dgv->Dock = DockStyle::Fill;
			dgv->AutoSizeColumnsMode = DataGridViewAutoSizeColumnsMode::Fill;
			dgv->ReadOnly = true;
			dgv->Columns->Add("Ma", L"Mã SP");
			dgv->Columns->Add("Ten", L"Tên SP");
			dgv->Columns->Add("Ton", L"Tồn Tại Kho (Tấn)");
			dgv->Columns->Add("ROP", L"Điểm Đặt Hàng");
			dgv->Columns->Add("EOQ", L"Gợi Ý Nhập (EOQ)");

			this->Controls->Add(dgv);
			this->Controls->Add(lbl);
		}

		void LoadData(List<CanhBaoItem^>^ list) {
			for each (auto item in list) {
				int idx = dgv->Rows->Add(item->MaSP, item->TenSP,
					item->TonTaiKho.ToString("N2"),
					item->DiemDatHang.ToString("N2"),
					item->EOQ.ToString("N2"));
				dgv->Rows[idx]->DefaultCellStyle->BackColor = Color::MistyRose;
			}
		}
	};
}