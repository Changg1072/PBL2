#pragma once
#include <vector>
#include <string>
#include <iostream>
#include <windows.h>
#include <sql.h>
#include <sqlext.h>
#include <msclr/marshal_cppstd.h>
#include "DataModels.h"
#include "QuanLyVanChuyen.h" 
#include <limits> 

using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections::Generic;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;
using namespace System::Drawing::Drawing2D;
using namespace System::Runtime::InteropServices;

namespace PBL2QuanLyKho {

	public ref class OrderConfirmationForm : public System::Windows::Forms::Form
	{
	public:
		// OUTPUT
		bool IsConfirmed = false;
		String^ SelectedShippingMethod = "";
		double FinalShippingCost = 0;
		double FinalTotal = 0;
		String^ MaYeuCauGoc = "";

	private:
		// INPUT
		List<CartItem^>^ _cartItems;
		Dictionary<String^, String^>^ _imageMap;
		String^ _maSieuThi;
		String^ _tenSieuThi;
		String^ _diaChiSieuThi;
		String^ _sdtSieuThi;
		SQLHDBC _hDbc;

		// UI CONTROLS
		Panel^ pnlHeader;
		Panel^ pnlFooter;
		Button^ btnConfirm;
		Label^ lblTotalFooter;

		Label^ lblDetailSubTotal;
		Label^ lblDetailShip;
		Label^ lblDetailFinal;

		FlowLayoutPanel^ pnlMainScroll;
		Panel^ pnlShipFast;
		Panel^ pnlShipStandard;

		// DATA VARS
		double costFast = 0;
		double costStd = 0;
		float timeFast = 0.0f;
		float timeStd = 0.0f;
		int selectedShipOption = 0;
		double _subTotal = 0;

	public:
		OrderConfirmationForm(
			List<CartItem^>^ cartItems,
			String^ maSieuThi,
			String^ tenST,
			String^ diaChiST,
			String^ sdtST,
			Dictionary<String^, String^>^ imgMap,
			System::IntPtr hDbcPtr,
			double fastCost, float fastTime,
			double stdCost, float stdTime,
			String^ maYeuCau
		);

	private:
		inline String^ ToSysString(std::wstring wstr) { return gcnew String(wstr.c_str()); }
		inline String^ ToSysString(std::string str) { return gcnew String(str.c_str()); }

		// Hàm chuyển đổi giờ phút
		String^ FormatTime(float totalHours);

		void InitializeComponent();
		void LoadData();

		// UI HELPERS
		Label^ CreateSectionTitle(String^ text);
		Panel^ CreateSectionPanel(int height);
		Panel^ CreateShippingOptionPanel(String^ name, String^ time, double cost, int id);
		Panel^ CreatePaymentDetailBox();
		Panel^ CreateFixedBottomPanel();

		// EVENTS & PAINTING
		void UpdateTotalUI();
		void OnResizeList(Object^ sender, EventArgs^ e);
		void DrawTopBorder(Object^ sender, PaintEventArgs^ e);
		void DrawRoundedPanel(Object^ sender, PaintEventArgs^ e);
		void DrawShippingPanel(Object^ sender, PaintEventArgs^ e);
		void DrawDecoLine(Object^ sender, PaintEventArgs^ e); // <--- Đã có mặt ở đây
		void OnDeactivate(Object^ sender, EventArgs^ e);
		void DrawFormBorder(Object^ sender, PaintEventArgs^ e);
		void btnBack_Click(Object^ sender, EventArgs^ e);
		void btnConfirm_Click(Object^ sender, EventArgs^ e);
		void OnShipOptionClick(Object^ sender, EventArgs^ e);
	};
}