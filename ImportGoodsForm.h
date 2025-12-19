#pragma once
#include "DataModels.h"
#include "ImportLogic.h"
#include "SaoLuu.h"
#include "Kho.h"          // <--- Thêm dòng này để hiểu "Kho"
#include "NhaCungCap.h"   // <--- Thêm dòng này để hiểu "NhaCungCap"
#include "LoHang.h"
#include "SanPham.h"         // <--- Đảm bảo file này tồn tại hoặc SanPham nằm trong DataModels.h
#include <msclr/marshal_cppstd.h>
#include <sqlext.h>
#include <sql.h>

namespace PBL2QuanLyKho {

    using namespace System;
    using namespace System::ComponentModel;
    using namespace System::Windows::Forms;
    using namespace System::Drawing;
    using namespace System::Collections::Generic;

    public ref class ImportGoodsForm : public System::Windows::Forms::Form
    {
    public:
        String^ MaKhoHienTai;

        // 1. Constructor CŨ (Giữ nguyên)
        ImportGoodsForm(String^ maKho) {
            this->MaKhoHienTai = maKho->Trim();
            InitializeComponent();
            currentCart = gcnew List<ImportItem^>();
            LoadProductsToUI();
        }

        // 2. [MỚI] Constructor NHẬN LIST TỰ ĐỘNG
        ImportGoodsForm(String^ maKho, List<ImportItem^>^ listThieu) {
            this->MaKhoHienTai = maKho->Trim();
            InitializeComponent();

            // Gán danh sách thiếu vào giỏ hàng ngay lập tức
            currentCart = listThieu;

            LoadProductsToUI();

            // Cập nhật giao diện giỏ hàng ngay
            RefreshCart();

            // Tự động chuyển sang Tab Giỏ hàng hoặc hiển thị thông báo
            MessageBox::Show(L"Đã tự động thêm " + listThieu->Count + L" sản phẩm đang thiếu vào giỏ!", L"Gợi ý nhập hàng");
        }
    private:
        Panel^ pnlLoading;

        void ShowLoading(bool show) {
            if (pnlLoading == nullptr) return;

            if (show) {
                pnlLoading->Visible = true;
                pnlLoading->BringToFront();
                Application::DoEvents(); // Vẽ lại UI ngay lập tức
            }
            else {
                pnlLoading->Visible = false;
            }
        }

        static String^ ToSysString(std::string s) {
            return gcnew String(s.c_str(), 0, (int)s.length(), System::Text::Encoding::UTF8);
        }
        // --- GUI CONTROLS ---
        TabControl^ tabControl;
        TabPage^ tabShop;

        FlowLayoutPanel^ flpProducts;
        DataGridView^ dgvCart;
        Label^ lblCartTotal;
        Button^ btnAnalyze;

        Panel^ pnlCheckout;
        FlowLayoutPanel^ flpLots;
        Label^ lblGrandTotal;
        Button^ btnConfirm;
        Button^ btnBack;

        // Data
        List<ImportItem^>^ currentCart;
        List<PredictedLot^>^ optimizedLots;
        Dictionary<String^, String^>^ imageMap;
        SQLWCHAR* GetConnectionString() {
            // Thêm ";Connection Timeout=3" vào cuối
            return (SQLWCHAR*)L"Driver={SQL Server};Server=LAPTOP-D0HRGU7N\\CHANGG2006;Database=PBL2;Trusted_Connection=yes;Connection Timeout=30;";
        }
        void InitializeComponent() {
            this->Text = L"Nhập Hàng & Tối Ưu Vận Chuyển (Kho: " + MaKhoHienTai + ")";
            this->Size = System::Drawing::Size(1300, 800);
            this->StartPosition = FormStartPosition::CenterScreen;

            tabControl = gcnew TabControl();
            tabControl->Dock = DockStyle::Fill;

            tabShop = gcnew TabPage(L"🛒 Mua Hàng & Tạo Đơn");
            InitializeShopUI();
            tabControl->Controls->Add(tabShop);

            this->Controls->Add(tabControl);
            pnlLoading = gcnew Panel();
            pnlLoading->Dock = DockStyle::Fill;
            pnlLoading->BackColor = Color::FromArgb(200, 0, 0, 0); // Màu đen mờ
            pnlLoading->Visible = false;

            Label^ lblWait = gcnew Label();
            lblWait->Text = L"⏳ Đang xử lý dữ liệu... Vui lòng đợi (Tối đa 30s)!";
            lblWait->ForeColor = Color::White;
            lblWait->Font = gcnew System::Drawing::Font(L"Segoe UI", 16, FontStyle::Bold);
            lblWait->AutoSize = true;
            lblWait->Location = Point(400, 350); // Căn giữa tương đối

            pnlLoading->Controls->Add(lblWait);
            this->Controls->Add(pnlLoading); // Thêm vào Form
            pnlLoading->BringToFront();
            // Overlay Checkout (Mặc định ẩn, hiện khi bấm Đặt hàng)
            InitializeCheckoutUI();
            this->Controls->Add(pnlCheckout);
        }

        void InitializeShopUI() {
            SplitContainer^ split = gcnew SplitContainer();
            split->Dock = DockStyle::Fill;
            split->FixedPanel = FixedPanel::Panel2;
            split->SplitterDistance = 700;

            // --- PHẦN BÊN TRÁI: DANH SÁCH SẢN PHẨM ---
            flpProducts = gcnew FlowLayoutPanel();
            flpProducts->Dock = DockStyle::Fill;
            flpProducts->AutoScroll = true;
            flpProducts->BackColor = Color::WhiteSmoke;
            flpProducts->Padding = System::Windows::Forms::Padding(10);
            split->Panel1->Controls->Add(flpProducts);

            // --- PHẦN BÊN PHẢI: GIỎ HÀNG ---
            Panel^ pnlCart = gcnew Panel();
            pnlCart->Dock = DockStyle::Fill;
            pnlCart->BackColor = Color::White;

            // 1. Tạo Tiêu Đề (Dock TOP)
            Label^ lblTitle = gcnew Label();
            lblTitle->Text = L"GIỎ HÀNG NHẬP";
            lblTitle->Dock = DockStyle::Top;
            lblTitle->Height = 50;
            lblTitle->TextAlign = ContentAlignment::MiddleCenter;
            lblTitle->Font = gcnew System::Drawing::Font(L"Segoe UI", 14, FontStyle::Bold);
            lblTitle->BackColor = Color::FromArgb(23, 42, 69);
            lblTitle->ForeColor = Color::White;

            // 2. Tạo Panel Đáy chứa Nút (Dock BOTTOM)
            Panel^ pnlBot = gcnew Panel();
            pnlBot->Dock = DockStyle::Bottom;
            pnlBot->Height = 100; // Đủ cao để chứa nút
            pnlBot->BackColor = Color::WhiteSmoke;

            // Label Tổng tiền
            lblCartTotal = gcnew Label();
            lblCartTotal->Text = L"Tổng: 0 VND";
            lblCartTotal->Font = gcnew System::Drawing::Font(L"Segoe UI", 12, FontStyle::Bold);
            lblCartTotal->ForeColor = Color::DarkRed;
            lblCartTotal->Location = Point(20, 10);
            lblCartTotal->AutoSize = true;

            // Nút "Tiến Hành"
            btnAnalyze = gcnew Button();
            btnAnalyze->Text = L"TIẾN HÀNH ĐẶT HÀNG ➤";
            btnAnalyze->Dock = DockStyle::Bottom; // Dính xuống đáy của pnlBot
            btnAnalyze->Height = 50;
            btnAnalyze->BackColor = Color::FromArgb(0, 114, 188);
            btnAnalyze->ForeColor = Color::White;
            btnAnalyze->Font = gcnew System::Drawing::Font(L"Segoe UI", 12, FontStyle::Bold);
            btnAnalyze->Cursor = Cursors::Hand;

            // [QUAN TRỌNG] Gắn sự kiện Click
            btnAnalyze->Click += gcnew EventHandler(this, &ImportGoodsForm::OnAnalyzeClick);

            // [QUAN TRỌNG] Thêm nút và label vào pnlBot
            pnlBot->Controls->Add(lblCartTotal);
            pnlBot->Controls->Add(btnAnalyze);

            // 3. Tạo GridView (Dock FILL)
            dgvCart = gcnew DataGridView();
            dgvCart->Dock = DockStyle::Fill;
            dgvCart->AutoSizeColumnsMode = DataGridViewAutoSizeColumnsMode::Fill;
            dgvCart->BackgroundColor = Color::White;
            dgvCart->Columns->Add("Ten", L"Sản Phẩm");
            dgvCart->Columns->Add("SL", L"SL (Tấn)");
            dgvCart->Columns->Add("ThanhTien", L"Thành Tiền");

            DataGridViewButtonColumn^ btnDel = gcnew DataGridViewButtonColumn();
            btnDel->HeaderText = L"Xóa"; btnDel->Text = L"🗑"; btnDel->UseColumnTextForButtonValue = true; btnDel->Width = 50;
            dgvCart->Columns->Add(btnDel);

            dgvCart->CellEndEdit += gcnew DataGridViewCellEventHandler(this, &ImportGoodsForm::OnCartEdit);
            dgvCart->CellContentClick += gcnew DataGridViewCellEventHandler(this, &ImportGoodsForm::OnCartClick);

            pnlCart->Controls->Clear();
            // Add theo thứ tự này để nút không bị che:
            pnlCart->Controls->Add(dgvCart);  // Bảng nằm dưới cùng
            pnlCart->Controls->Add(pnlBot);   // Nút nằm đè lên trên bảng
            pnlCart->Controls->Add(lblTitle); // Tiêu đề nằm trên cùng
            pnlBot->BringToFront();

            // Đảm bảo chắc chắn bằng lệnh (phòng khi copy paste sai thứ tự):
            pnlCart->Controls->Add(dgvCart);  // Add Fill trước
            pnlCart->Controls->Add(pnlBot);   // Add Bottom
            pnlCart->Controls->Add(lblTitle);// Lấp đầy phần còn lại (Fill)

            split->Panel2->Controls->Add(pnlCart);
            tabShop->Controls->Add(split);
        }

        void InitializeCheckoutUI() {
            pnlCheckout = gcnew Panel();
            pnlCheckout->Dock = DockStyle::Fill;
            pnlCheckout->BackColor = Color::White;
            pnlCheckout->Visible = false; // Ẩn lúc đầu

            Label^ lblHeader = gcnew Label(); lblHeader->Text = L"📦 XÁC NHẬN CHIA LÔ & VẬN CHUYỂN";
            lblHeader->Dock = DockStyle::Top; lblHeader->Height = 60;
            lblHeader->TextAlign = ContentAlignment::MiddleCenter;
            lblHeader->Font = gcnew System::Drawing::Font(L"Segoe UI", 16, FontStyle::Bold);
            lblHeader->BackColor = Color::FromArgb(240, 240, 240);

            flpLots = gcnew FlowLayoutPanel();
            flpLots->Dock = DockStyle::Fill;
            flpLots->AutoScroll = true;
            flpLots->Padding = System::Windows::Forms::Padding(50, 20, 50, 20);

            Panel^ pnlAction = gcnew Panel(); pnlAction->Dock = DockStyle::Bottom; pnlAction->Height = 80;
            pnlAction->BackColor = Color::WhiteSmoke;

            btnBack = gcnew Button(); btnBack->Text = L"⬅ Quay lại";
            btnBack->Location = Point(50, 20); btnBack->Size = System::Drawing::Size(150, 40);
            btnBack->Click += gcnew EventHandler(this, &ImportGoodsForm::OnBackClick);

            btnConfirm = gcnew Button(); btnConfirm->Text = L"✅ XÁC NHẬN NHẬP KHO";
            btnConfirm->BackColor = Color::Green; btnConfirm->ForeColor = Color::White;
            btnConfirm->Font = gcnew System::Drawing::Font(L"Segoe UI", 12, FontStyle::Bold);
            btnConfirm->Dock = DockStyle::Right; btnConfirm->Width = 300;
            btnConfirm->Click += gcnew EventHandler(this, &ImportGoodsForm::OnConfirmClick);

            lblGrandTotal = gcnew Label(); lblGrandTotal->Text = L"Tổng cộng: ...";
            lblGrandTotal->Location = Point(250, 25); lblGrandTotal->AutoSize = true;
            lblGrandTotal->Font = gcnew System::Drawing::Font(L"Segoe UI", 14, FontStyle::Bold);
            lblGrandTotal->ForeColor = Color::DarkRed;

            pnlAction->Controls->Add(btnBack);
            pnlAction->Controls->Add(lblGrandTotal);
            pnlAction->Controls->Add(btnConfirm);

            pnlCheckout->Controls->Add(flpLots);
            pnlCheckout->Controls->Add(pnlAction);
            pnlCheckout->Controls->Add(lblHeader);
        }

        // --- LOAD DỮ LIỆU ---
        void LoadProductsToUI() {
            LoadImageMap(); // Đọc file ảnh

            SQLHENV hEnv; SQLHDBC hDbc; SQLHSTMT hStmt;
            SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv);
            SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
            SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc);
            SQLDriverConnect(hDbc, NULL, GetConnectionString(), SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);

            std::vector<SanPham> listSP;
            DocfileSanPham(hDbc, listSP); // Dùng hàm có sẵn trong SaoLuu.cpp

            flpProducts->Controls->Clear();
            for (const auto& sp : listSP) {
                CreateProductCard(sp);
            }

            SQLDisconnect(hDbc); SQLFreeHandle(SQL_HANDLE_DBC, hDbc); SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
        }

        void LoadImageMap() {
            imageMap = gcnew Dictionary<String^, String^>();
            String^ fileName = "ProductImages.txt";
            if (System::IO::File::Exists(fileName)) {
                cli::array<String^>^ lines = System::IO::File::ReadAllLines(fileName);
                for each (String ^ line in lines) {
                    cli::array<String^>^ parts = line->Split('|');
                    if (parts->Length >= 2) imageMap[parts[0]->Trim()] = parts[1]->Trim()->Replace("\\\\", "\\");
                }
            }
        }
        // 1. Hàm CreateProductCard đã sửa
        void CreateProductCard(const SanPham& sp) {
            Panel^ card = gcnew Panel();
            card->Size = System::Drawing::Size(220, 340);
            card->BackColor = Color::White;
            card->Margin = System::Windows::Forms::Padding(10);
            card->BorderStyle = BorderStyle::FixedSingle;

            // 1. Hình ảnh
            PictureBox^ pb = gcnew PictureBox();
            pb->Size = System::Drawing::Size(200, 150);
            pb->Location = Point(10, 10);
            pb->SizeMode = PictureBoxSizeMode::Zoom;

            String^ maSP = ToSysString(sp.getMaSP())->Trim();
            if (imageMap->ContainsKey(maSP)) {
                try { pb->Image = Image::FromFile(imageMap[maSP]); }
                catch (...) {}
            }

            // 2. Tên SP
            Label^ lblName = gcnew Label();
            lblName->Text = ToSysString(sp.getTenSP());
            lblName->Location = Point(10, 170);
            lblName->Size = System::Drawing::Size(200, 45);
            lblName->Font = gcnew System::Drawing::Font(L"Segoe UI", 10, FontStyle::Bold);

            // 3. Giá Vốn
            Label^ lblPrice = gcnew Label();
            lblPrice->Text = L"Giá: " + sp.getGiaVon().ToString("N0") + L" đ";
            lblPrice->Location = Point(10, 220);
            lblPrice->AutoSize = true;
            lblPrice->ForeColor = Color::DimGray;

            // 4. Ô nhập số lượng (QUAN TRỌNG: Phải đặt đúng Tên để hàm Add tìm thấy)
            Label^ lblQtyTitle = gcnew Label();
            lblQtyTitle->Text = L"Số lượng (Tấn):";
            lblQtyTitle->Location = Point(10, 250);
            lblQtyTitle->AutoSize = true;

            NumericUpDown^ numQty = gcnew NumericUpDown();
            numQty->Name = "numQty"; // <--- BẮT BUỘC PHẢI CÓ DÒNG NÀY
            numQty->Location = Point(110, 248);
            numQty->Size = System::Drawing::Size(90, 25);
            numQty->Minimum = 1;
            numQty->Maximum = 9999;
            numQty->Value = 1;

            // 5. Nút Thêm
            Button^ btnAdd = gcnew Button();
            btnAdd->Text = L"Thêm vào giỏ";
            btnAdd->Location = Point(10, 290);
            btnAdd->Size = System::Drawing::Size(200, 35);
            btnAdd->BackColor = Color::Orange;
            btnAdd->ForeColor = Color::White;
            btnAdd->FlatStyle = FlatStyle::Flat;
            btnAdd->Font = gcnew System::Drawing::Font(L"Segoe UI", 10, FontStyle::Bold);
            btnAdd->Cursor = Cursors::Hand;

            // Lưu dữ liệu vào nút
            ImportItem^ item = gcnew ImportItem();
            item->MaSP = maSP;
            item->TenSP = lblName->Text;
            item->GiaVon = sp.getGiaVon();
            item->SoLuong = 1; // Mặc định, sẽ cập nhật lại khi bấm

            btnAdd->Tag = item;
            btnAdd->Click += gcnew EventHandler(this, &ImportGoodsForm::OnAddToCart);

            card->Controls->Add(lblQtyTitle);
            card->Controls->Add(numQty);
            card->Controls->Add(btnAdd);
            card->Controls->Add(lblPrice);
            card->Controls->Add(lblName);
            card->Controls->Add(pb);

            flpProducts->Controls->Add(card);
        }

        void OnAddToCart(Object^ sender, EventArgs^ e) {
            Button^ btn = dynamic_cast<Button^>(sender);
            if (btn == nullptr) return;

            ImportItem^ tagItem = dynamic_cast<ImportItem^>(btn->Tag);
            if (tagItem == nullptr) return;

            // 1. Lấy số lượng từ ô NumericUpDown cùng thẻ
            double qtyToAdd = 1;
            if (btn->Parent != nullptr) {
                // Tìm control có tên "numQty" trong cùng thẻ cha
                cli::array<Control^>^ foundControls = btn->Parent->Controls->Find("numQty", false);
                if (foundControls->Length > 0) {
                    NumericUpDown^ nud = dynamic_cast<NumericUpDown^>(foundControls[0]);
                    if (nud != nullptr) {
                        qtyToAdd = (double)nud->Value;
                    }
                }
            }

            // 2. Logic thêm vào giỏ hàng
            bool found = false;
            for each (ImportItem ^ i in currentCart) {
                // So sánh Mã SP (Trim để xóa khoảng trắng thừa)
                if (String::Compare(i->MaSP->Trim(), tagItem->MaSP->Trim(), true) == 0) {
                    i->SoLuong += qtyToAdd;
                    found = true;
                    break;
                }
            }

            // 3. Nếu chưa có thì tạo mới
            if (!found) {
                ImportItem^ newItem = gcnew ImportItem();
                newItem->MaSP = tagItem->MaSP->Trim();
                newItem->TenSP = tagItem->TenSP;
                newItem->GiaVon = tagItem->GiaVon;
                newItem->SoLuong = qtyToAdd;
                currentCart->Add(newItem);
            }

            RefreshCart();
        }
        void RefreshCart() {
            dgvCart->Rows->Clear();
            double total = 0;
            for each (ImportItem ^ i in currentCart) {
                dgvCart->Rows->Add(i->TenSP, i->SoLuong, i->ThanhTien().ToString("N0"));
                total += i->ThanhTien();
            }
            lblCartTotal->Text = L"Tổng tiền hàng: " + total.ToString("N0") + " VND";
        }

        void OnCartEdit(Object^ sender, DataGridViewCellEventArgs^ e) {
            if (e->RowIndex < 0) return;
            if (e->ColumnIndex == 1) { // Sửa số lượng
                double sl = 0;
                if (Double::TryParse(dgvCart->Rows[e->RowIndex]->Cells[1]->Value->ToString(), sl)) {
                    currentCart[e->RowIndex]->SoLuong = sl;
                    RefreshCart();
                }
            }
        }

        void OnCartClick(Object^ sender, DataGridViewCellEventArgs^ e) {
            if (e->RowIndex >= 0 && e->ColumnIndex == 3) { // Xóa
                currentCart->RemoveAt(e->RowIndex);
                RefreshCart();
            }
        }
        /*
        // --- HÀM LƯU DỮ LIỆU NHẬP KHO TRỰC TIẾP (ĐÃ CHỈNH SỬA THEO ẢNH DB MỚI NHẤT) ---
        void SaveImportToDB() {
            if (optimizedLots == nullptr || optimizedLots->Count == 0) {
                MessageBox::Show(L"Chưa có lô hàng nào để nhập!", L"Thông báo");
                return;
            }

            SQLHENV hEnv; SQLHDBC hDbc; SQLHSTMT hStmt = SQL_NULL_HSTMT;
            if (!SQL_SUCCESS(SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv))) return;
            SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
            SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc);

            SQLDriverConnect(hDbc, NULL, GetConnectionString(), SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);
            SQLSetConnectAttr(hDbc, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER)SQL_AUTOCOMMIT_OFF, 0); // Bắt đầu Transaction

            try {
                for each (PredictedLot ^ lot in optimizedLots) {
                    String^ maLo = "LO" + DateTime::Now.Ticks.ToString()->Substring(8);
                    double tongTan = 0;
                    double tienHang = lot->TongTienHang();
                    for each (ImportItem ^ i in lot->Items) tongTan += i->SoLuong;

                    // ---------------------------------------------------------
                    // 1. BẢNG LoHang (Theo ảnh image_b25b74.png)
                    // Cột: MaLoHang, MaKho, MaNCC, NgayNhap, SoLuongTanTong, GiaTriTong
                    // (Lưu ý: Không có cột TongTheTich)
                    // ---------------------------------------------------------
                    if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {
                        SQLWCHAR* sql = (SQLWCHAR*)L"INSERT INTO LoHang (MaLoHang, MaKho, MaNCC, NgayNhap, SoLuongTanTong, GiaTriTong) VALUES (?, ?, ?, GETDATE(), ?, ?)";
                        SQLPrepare(hStmt, sql, SQL_NTS);

                        std::string sMaLo = ToStdString(maLo);
                        std::string sMaKho = ToStdString(this->MaKhoHienTai);
                        std::string sMaNCC = ToStdString(lot->MaNCC);

                        SQLCHAR cMaLo[256], cMaKho[256], cMaNCC[256];
                        strcpy_s((char*)cMaLo, 256, sMaLo.c_str());
                        strcpy_s((char*)cMaKho, 256, sMaKho.c_str());
                        strcpy_s((char*)cMaNCC, 256, sMaNCC.c_str());

                        SQLDOUBLE dTan = tongTan;
                        SQLDOUBLE dVal = tienHang + lot->PhiVanChuyen;

                        SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 255, 0, cMaLo, 0, NULL);
                        SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 255, 0, cMaKho, 0, NULL);
                        SQLBindParameter(hStmt, 3, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 255, 0, cMaNCC, 0, NULL);
                        SQLBindParameter(hStmt, 4, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DECIMAL, 18, 2, &dTan, 0, NULL);
                        SQLBindParameter(hStmt, 5, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DECIMAL, 18, 2, &dVal, 0, NULL);

                        if (SQLExecute(hStmt) != SQL_SUCCESS) throw gcnew Exception("Lỗi Insert LoHang");
                        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
                    }

                    // ---------------------------------------------------------
                    // 2. BẢNG TonKho (Theo ảnh image_b25b70.png)
                    // Cột: MaLoHang, MaKho, SoLuongTanTong
                    // ---------------------------------------------------------
                    if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {
                        SQLWCHAR* sql = (SQLWCHAR*)L"INSERT INTO TonKho (MaLoHang, MaKho, SoLuongTanTong) VALUES (?, ?, ?)";
                        SQLPrepare(hStmt, sql, SQL_NTS);

                        std::string sMaLo = ToStdString(maLo);
                        std::string sMaKho = ToStdString(this->MaKhoHienTai);
                        SQLCHAR cMaLo[256], cMaKho[51];
                        strcpy_s((char*)cMaLo, 256, sMaLo.c_str());
                        strcpy_s((char*)cMaKho, 51, sMaKho.c_str());
                        SQLDOUBLE dTan = tongTan;

                        SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 255, 0, cMaLo, 0, NULL);
                        SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, cMaKho, 0, NULL);
                        SQLBindParameter(hStmt, 3, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_FLOAT, 0, 0, &dTan, 0, NULL);

                        if (SQLExecute(hStmt) != SQL_SUCCESS) throw gcnew Exception("Lỗi Insert TonKho");
                        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
                    }

                    // --- VÒNG LẶP CHI TIẾT ---
                    for each (ImportItem ^ item in lot->Items) {
                        std::string sMaLo = ToStdString(maLo);
                        std::string sMaSP = ToStdString(item->MaSP);
                        SQLCHAR cMaLo[51], cMaSP[51];
                        strcpy_s((char*)cMaLo, 51, sMaLo.c_str());
                        strcpy_s((char*)cMaSP, 51, sMaSP.c_str());

                        SQLDOUBLE dSL = item->SoLuong;
                        SQLDOUBLE dGia = item->ThanhTien();
                        SQLDOUBLE dVol = item->SoLuong; // Tạm tính thể tích
                        DateTime dt = DateTime::Now.AddDays(180);
                        SQL_DATE_STRUCT dHSD = { dt.Year, dt.Month, dt.Day };

                        // ---------------------------------------------------------
                        // 3. BẢNG ChiTietTonKho (Theo ảnh image_b25b6c.png)
                        // Cột: MaLoHang, MaSanPham, SoLuongTan, HanSuDung, GiaTriSanPham, TheTich
                        // (Bỏ cột S vì là tự tăng, tên cột giá là GiaTriSanPham)
                        // ---------------------------------------------------------
                        if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {
                            SQLWCHAR* sql = (SQLWCHAR*)L"INSERT INTO ChiTietTonKho (MaLoHang, MaSanPham, SoLuongTan, HanSuDung, GiaTriSanPham, TheTich) VALUES (?, ?, ?, ?, ?, ?)";
                            SQLPrepare(hStmt, sql, SQL_NTS);

                            SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 255, 0, cMaLo, 0, NULL);
                            SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 255, 0, cMaSP, 0, NULL);
                            SQLBindParameter(hStmt, 3, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_FLOAT, 0, 0, &dSL, 0, NULL);
                            SQLBindParameter(hStmt, 4, SQL_PARAM_INPUT, SQL_C_TYPE_DATE, SQL_DATE, 0, 0, &dHSD, 0, NULL);
                            SQLBindParameter(hStmt, 5, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DECIMAL, 18, 2, &dGia, 0, NULL);
                            SQLBindParameter(hStmt, 6, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_FLOAT, 0, 0, &dVol, 0, NULL);

                            if (SQLExecute(hStmt) != SQL_SUCCESS) throw gcnew Exception("Lỗi Insert ChiTietTonKho");
                            SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
                        }

                        // ---------------------------------------------------------
                        // 4. BẢNG ChiTietLoHang (Theo ảnh image_b25e37.png)
                        // Cột: MaLoHang, MaSanPham, SoLuongTan, HanSuDung, GiaTriSP, TheTich
                        // (Lưu ý: Tên cột giá ở đây là GiaTriSP, KHÁC với bảng trên)
                        // ---------------------------------------------------------
                        if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {
                            SQLWCHAR* sql = (SQLWCHAR*)L"INSERT INTO ChiTietLoHang (MaLoHang, MaSanPham, SoLuongTan, HanSuDung, GiaTriSP, TheTich) VALUES (?, ?, ?, ?, ?, ?)";
                            SQLPrepare(hStmt, sql, SQL_NTS);

                            SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 255, 0, cMaLo, 0, NULL);
                            SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 255, 0, cMaSP, 0, NULL);
                            SQLBindParameter(hStmt, 3, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DECIMAL, 18, 2, &dSL, 0, NULL);
                            SQLBindParameter(hStmt, 4, SQL_PARAM_INPUT, SQL_C_TYPE_DATE, SQL_DATE, 0, 0, &dHSD, 0, NULL);
                            SQLBindParameter(hStmt, 5, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DECIMAL, 18, 2, &dGia, 0, NULL);
                            SQLBindParameter(hStmt, 6, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DECIMAL, 18, 2, &dVol, 0, NULL);

                            if (SQLExecute(hStmt) != SQL_SUCCESS) throw gcnew Exception("Lỗi Insert ChiTietLoHang");
                            SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
                        }
                    }
                }

                SQLEndTran(SQL_HANDLE_DBC, hDbc, SQL_COMMIT);
                MessageBox::Show(L"✅ Nhập kho thành công!", L"Hoàn tất");
                this->DialogResult = System::Windows::Forms::DialogResult::OK;
                this->Close();
            }
            catch (Exception^ ex) {
                SQLEndTran(SQL_HANDLE_DBC, hDbc, SQL_ROLLBACK);
                MessageBox::Show(L"❌ Lỗi SQL: " + ex->Message);
            }

            SQLSetConnectAttr(hDbc, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER)SQL_AUTOCOMMIT_ON, 0);
            SQLDisconnect(hDbc); SQLFreeHandle(SQL_HANDLE_DBC, hDbc); SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
        }
        */
        // Helper: Chuyển String^ sang std::string (Nếu bạn chưa có hàm này trong file)
        std::string ToStdString(String^ s) {
            if (String::IsNullOrEmpty(s)) return "";
            msclr::interop::marshal_context context;
            return context.marshal_as<std::string>(s);
        }

        // --- CHIA LÔ ---
        // --- SỰ KIỆN NÚT "TIẾN HÀNH ĐẶT HÀNG" ---
        // --- SỰ KIỆN NÚT "TIẾN HÀNH ĐẶT HÀNG" (ĐÃ SỬA LỖI & THÊM DEBUG) ---
        void OnAnalyzeClick(Object^ sender, EventArgs^ e) {
            // 1. Validation: Kiểm tra giỏ hàng
            if (currentCart == nullptr || currentCart->Count == 0) {
                MessageBox::Show(L"Giỏ hàng đang trống! Vui lòng chọn sản phẩm trước.", L"Cảnh báo", MessageBoxButtons::OK, MessageBoxIcon::Warning);
                return;
            }

            // [MỚI] Bật màn hình Loading thay vì chỉ xoay chuột
            // Lý do: Kết nối có thể mất tới 30s, xoay chuột người dùng tưởng máy treo
            ShowLoading(true);

            SQLHENV hEnv = SQL_NULL_HENV;
            SQLHDBC hDbc = SQL_NULL_HDBC;

            // Cấp phát Environment
            if (SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv) != SQL_SUCCESS) {
                ShowLoading(false);
                return;
            }
            SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);

            // Cấp phát Connection Handle
            if (SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc) != SQL_SUCCESS) {
                SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
                ShowLoading(false);
                return;
            }

            // 2. Kết nối CSDL (Sử dụng Timeout trong chuỗi kết nối)
            // Đảm bảo hàm GetConnectionString() của bạn đã có ";Connection Timeout=30;"
            SQLRETURN ret = SQLDriverConnect(hDbc, NULL, GetConnectionString(), SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);

            // [QUAN TRỌNG] Kiểm tra kết nối nghiêm ngặt
            if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
                ShowLoading(false); // Tắt Loading ngay
                MessageBox::Show(L"❌ LỖI KẾT NỐI SQL!\nKhông thể kết nối đến Database (Timeout hoặc Sai Server).\n\nHãy kiểm tra lại mạng hoặc tên máy chủ.", L"Lỗi Kết Nối", MessageBoxButtons::OK, MessageBoxIcon::Error);

                // Giải phóng bộ nhớ ngay lập tức
                SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
                SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
                return;
            }

            try {
                // --- GỌI THUẬT TOÁN TỐI ƯU ---
                // (Đã được tối ưu Batch Query trong file ImportLogic.h)
                optimizedLots = ImportOptimizer::OptimizeCart(hDbc, currentCart, this->MaKhoHienTai);

                // 3. Hiển thị kết quả ra UI
                flpLots->Controls->Clear();
                double totalAll = 0;
                bool hasError = false;

                if (optimizedLots == nullptr || optimizedLots->Count == 0) {
                    MessageBox::Show(L"Không tính toán được phương án nào (Dữ liệu Xe hoặc NCC bị thiếu).", L"Thông báo");
                }
                else {
                    int idx = 1;
                    for each (PredictedLot ^ lot in optimizedLots) {
                        GroupBox^ gb = gcnew GroupBox();

                        // Xử lý Null an toàn
                        String^ tenNCC = (lot->TenNCC != nullptr) ? lot->TenNCC : L"NCC Lẻ";
                        String^ lyDo = (lot->LyDoChon != nullptr) ? lot->LyDoChon : L"Unknown";

                        gb->Text = L"LÔ #" + idx + L": " + tenNCC + L" | " + lyDo;
                        gb->Width = 850;
                        gb->Height = 160;
                        gb->Font = gcnew System::Drawing::Font(L"Segoe UI", 11, FontStyle::Bold);
                        gb->BackColor = Color::WhiteSmoke;

                        // Label 1: Tuyến đường
                        Label^ lblRoute = gcnew Label();
                        lblRoute->Text = L"Lộ trình: " + ((lot->GhiChuRoute != nullptr) ? lot->GhiChuRoute : L"Đường bộ");
                        lblRoute->Location = Point(20, 30);
                        lblRoute->AutoSize = true;
                        lblRoute->ForeColor = Color::DarkBlue;

                        // Label 2: Chi phí
                        Label^ lblShip = gcnew Label();
                        bool isShipError = (lot->PhiVanChuyen > 1000000000); // Check logic lỗi giá

                        if (isShipError) {
                            lblShip->Text = L"⚠️ Không tìm thấy xe phù hợp cho khối lượng này!";
                            lblShip->ForeColor = Color::Red;
                            hasError = true;
                        }
                        else {
                            lblShip->Text = L"Phí Ship: " + lot->PhiVanChuyen.ToString("N0") + L" đ | Thời gian: " + lot->ThoiGianDuKien.ToString("N1") + L"h";
                            lblShip->ForeColor = Color::Green;
                        }
                        lblShip->Location = Point(20, 60);
                        lblShip->AutoSize = true;

                        // Label 3: Tiền hàng
                        Label^ lblGoodsVal = gcnew Label();
                        lblGoodsVal->Text = L"Tiền Hàng: " + lot->TongTienHang().ToString("N0") + L" đ";
                        lblGoodsVal->Location = Point(20, 90);
                        lblGoodsVal->AutoSize = true;
                        lblGoodsVal->ForeColor = Color::Black;

                        gb->Controls->Add(lblRoute);
                        gb->Controls->Add(lblShip);
                        gb->Controls->Add(lblGoodsVal);

                        flpLots->Controls->Add(gb);

                        // Chỉ cộng tổng nếu phí hợp lệ
                        if (!isShipError)
                            totalAll += lot->TongTienHang() + lot->PhiVanChuyen;

                        idx++;
                    }
                }

                lblGrandTotal->Text = L"TỔNG THANH TOÁN: " + totalAll.ToString("N0") + L" VNĐ";

                if (hasError) {
                    MessageBox::Show(L"⚠️ Một số lô hàng không tìm thấy xe vận chuyển phù hợp.\nVui lòng kiểm tra lại.", L"Cảnh báo vận chuyển", MessageBoxButtons::OK, MessageBoxIcon::Warning);
                }

                // Mở Panel Overlay Checkout
                pnlCheckout->Visible = true;
                pnlCheckout->BringToFront();

            }
            catch (Exception^ ex) {
                MessageBox::Show(L"Lỗi tính toán logic: " + ex->Message + L"\n\nStack: " + ex->StackTrace, L"Crash App");
            }
            finally {
                // [QUAN TRỌNG] Luôn luôn chạy vào đây để dọn dẹp

                if (hDbc != SQL_NULL_HDBC) {
                    SQLDisconnect(hDbc);
                    SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
                }
                if (hEnv != SQL_NULL_HENV) {
                    SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
                }

                ShowLoading(false); // Tắt màn hình chờ
            }
        }
        /*
        void OnConfirmImportClick(Object^ sender, EventArgs^ e) {
            // Gọi hàm lưu vào DB
            SaveImportToDB();
        }
        */
        void DisplayLots() {
            flpLots->Controls->Clear();
            double totalAll = 0;

            int idx = 1;
            for each (PredictedLot ^ lot in optimizedLots) {
                GroupBox^ gb = gcnew GroupBox();
                gb->Text = L"LÔ #" + idx + L": " + lot->TenNCC + L" (" + lot->LyDoChon + L")";
                gb->Width = 1000;
                gb->Height = 150 + (lot->Items->Count * 30);
                gb->Font = gcnew System::Drawing::Font(L"Segoe UI", 11, FontStyle::Bold);

                Label^ lblShip = gcnew Label();
                lblShip->Text = L"🚚 Phí Ship: " + lot->PhiVanChuyen.ToString("N0") + L" | ⏱ Thời gian: " + lot->ThoiGianDuKien + "h";
                lblShip->Location = Point(20, 30); lblShip->AutoSize = true; lblShip->ForeColor = Color::Blue;
                gb->Controls->Add(lblShip);

                int y = 60;
                for each (ImportItem ^ i in lot->Items) {
                    Label^ lItem = gcnew Label();
                    lItem->Text = L"• " + i->TenSP + L" | SL: " + i->SoLuong + L" tấn | " + i->ThanhTien().ToString("N0");
                    lItem->Location = Point(30, y); lItem->AutoSize = true; lItem->Font = gcnew System::Drawing::Font(L"Segoe UI", 10, FontStyle::Regular);
                    gb->Controls->Add(lItem);
                    y += 25;
                }

                flpLots->Controls->Add(gb);
                totalAll += lot->TongTienHang() + lot->PhiVanChuyen;
                idx++;
            }
            lblGrandTotal->Text = L"Tổng thanh toán: " + totalAll.ToString("N0") + L" VND";
        }

        void OnBackClick(Object^ sender, EventArgs^ e) { pnlCheckout->Visible = false; }
        void ShowSQLError(SQLHANDLE hHandle, SQLSMALLINT hType, String^ place) {
            SQLWCHAR sqlState[6];
            SQLWCHAR message[1024];
            SQLINTEGER nativeError;
            SQLSMALLINT textLength;

            SQLRETURN ret = SQLGetDiagRec(hType, hHandle, 1, sqlState, &nativeError, message, 1024, &textLength);

            if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
                MessageBox::Show(
                    L"❌ Lỗi SQL tại: " + place + L"\n\n" +
                    L"State: " + gcnew String(sqlState) + L"\n" +
                    L"Code: " + nativeError + L"\n" +
                    L"Message: " + gcnew String(message),
                    L"SQL Error",
                    MessageBoxButtons::OK,
                    MessageBoxIcon::Error
                );
            }
            else {
                // Nếu không lấy được lỗi chi tiết, hiện thông báo chung
                MessageBox::Show(
                    L"❌ Lỗi SQL không xác định tại: " + place + L"\n\nKiểm tra:\n" +
                    L"1. Cấu trúc bảng có đúng?\n" +
                    L"2. Kiểu dữ liệu có khớp?\n" +
                    L"3. Có vi phạm Constraint nào không?",
                    L"Unknown SQL Error",
                    MessageBoxButtons::OK,
                    MessageBoxIcon::Warning
                );
            }
        }

        void OnConfirmClick(Object^ sender, EventArgs^ e) {
            // 1. Kiểm tra đầu vào
            if (optimizedLots == nullptr || optimizedLots->Count == 0) {
                MessageBox::Show(L"Không có lô hàng nào để nhập! Vui lòng ấn 'Tiến Hành' trước.", L"Cảnh báo");
                return;
            }

            // 2. Setup Giao diện Loading
            ShowLoading(true);
            Application::DoEvents();
            btnConfirm->Enabled = false;

            // 3. Khai báo biến SQL
            SQLHENV hEnv = SQL_NULL_HENV;
            SQLHDBC hDbc = SQL_NULL_HDBC;
            SQLHSTMT hStmt = SQL_NULL_HSTMT;

            try {
                // --- KHỞI TẠO KẾT NỐI ---
                if (SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv) != SQL_SUCCESS) throw gcnew Exception("Lỗi Env");
                SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);

                if (SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc) != SQL_SUCCESS) throw gcnew Exception("Lỗi Dbc");
                SQLSetConnectAttr(hDbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER)10, 0);

                SQLRETURN ret = SQLDriverConnect(hDbc, NULL, GetConnectionString(), SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);
                if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
                    throw gcnew Exception("Không thể kết nối Database");
                }

                // BẮT ĐẦU TRANSACTION (Quan trọng: Để nếu lỗi thì rollback hết)
                SQLSetConnectAttr(hDbc, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER)SQL_AUTOCOMMIT_OFF, 0);

                // Biến dùng chung
                int count = 0;
                String^ baseTime = DateTime::Now.ToString("yyMMddHHmmss"); // Lấy thời gian hiện tại làm gốc

                // -----------------------------------------------------------
                // [QUAN TRỌNG] VÒNG LẶP CHÍNH: DUYỆT QUA TỪNG LÔ ĐỂ TẠO ĐƠN
                // -----------------------------------------------------------
                for each(PredictedLot ^ lot in optimizedLots) {
                    count++;
                    // Tạo mã đơn riêng biệt: VD DH231215..._1, DH231215..._2
                    String^ maDon = "DH" + baseTime + "_" + count;

                    // =======================================================
                    // BƯỚC A: INSERT BẢNG DonDatHang (Lưu NCC và Xe)
                    // =======================================================
                    if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {
                        // Câu lệnh SQL đã thêm MaNCC và MaXe
                        std::wstring sql = L"INSERT INTO DonDatHang (MaDonHang, NgayDat, MaKho, TongTienHang, TongPhiVanChuyen, TongThanhToan, TrangThai, MaNCC, MaXe) VALUES (?, GETDATE(), ?, ?, ?, ?, ?, ?, ?)";
                        SQLPrepare(hStmt, (SQLWCHAR*)sql.c_str(), SQL_NTS);

                        // Chuẩn bị dữ liệu
                        std::string sMaDon = ToStdString(maDon);
                        std::string sMaKho = ToStdString(this->MaKhoHienTai);
                        std::string sMaNCC = ToStdString(lot->MaNCC); // <--- Lấy NCC của Lô này
                        std::string sMaXe = ToStdString(lot->MaXe);  // <--- Lấy Xe của Lô này
                        std::wstring wsTrangThai = L"Đang Vận Chuyển";

                        SQLDOUBLE dTienHang = lot->TongTienHang();
                        SQLDOUBLE dPhiShip = lot->PhiVanChuyen;
                        SQLDOUBLE dTongTT = dTienHang + dPhiShip;

                        // Bind tham số
                        SQLCHAR cMaDon[51], cMaKho[51], cMaNCC[256], cMaXe[51];
                        SQLWCHAR wcTrangThai[51];

                        strcpy_s((char*)cMaDon, 51, sMaDon.c_str());
                        strcpy_s((char*)cMaKho, 51, sMaKho.c_str());
                        strcpy_s((char*)cMaNCC, 256, sMaNCC.c_str());
                        strcpy_s((char*)cMaXe, 51, sMaXe.c_str());
                        wcsncpy_s(wcTrangThai, 51, wsTrangThai.c_str(), _TRUNCATE);

                        SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, cMaDon, 0, NULL);
                        SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, cMaKho, 0, NULL);
                        SQLBindParameter(hStmt, 3, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DECIMAL, 18, 2, &dTienHang, 0, NULL);
                        SQLBindParameter(hStmt, 4, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DECIMAL, 18, 2, &dPhiShip, 0, NULL);
                        SQLBindParameter(hStmt, 5, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DECIMAL, 18, 2, &dTongTT, 0, NULL);
                        SQLBindParameter(hStmt, 6, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 50, 0, wcTrangThai, 0, NULL);
                        SQLBindParameter(hStmt, 7, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 255, 0, cMaNCC, 0, NULL); // MaNCC
                        SQLBindParameter(hStmt, 8, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, cMaXe, 0, NULL);   // MaXe

                        if (SQLExecute(hStmt) != SQL_SUCCESS) {
                            ShowSQLError(hStmt, SQL_HANDLE_STMT, "Insert DonDatHang: " + maDon);
                            throw gcnew Exception("Lỗi tạo đơn hàng chính");
                        }
                        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
                    }

                    // =======================================================
                    // BƯỚC B: INSERT BẢNG ChiTietDonDatHang (Chi tiết SP)
                    // =======================================================
                    if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {
                        std::wstring sqlDet = L"INSERT INTO ChiTietDonDatHang (MaDonHang, MaSanPham, SoLuong, DonGiaVon, ThanhTien) VALUES (?, ?, ?, ?, ?)";
                        SQLPrepare(hStmt, (SQLWCHAR*)sqlDet.c_str(), SQL_NTS);

                        // Bind biến cố định (MaDonHang)
                        std::string sMD = ToStdString(maDon);
                        SQLCHAR cMD[51]; strcpy_s((char*)cMD, 51, sMD.c_str());
                        SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, cMD, 0, NULL);

                        // Bind biến thay đổi (SanPham)
                        SQLCHAR cSP[256];
                        SQLDOUBLE dSL, dGia, dTT;
                        SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 255, 0, cSP, 0, NULL);
                        SQLBindParameter(hStmt, 3, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_FLOAT, 0, 0, &dSL, 0, NULL);
                        SQLBindParameter(hStmt, 4, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DECIMAL, 18, 2, &dGia, 0, NULL);
                        SQLBindParameter(hStmt, 5, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DECIMAL, 18, 2, &dTT, 0, NULL);

                        // [QUAN TRỌNG] Chỉ duyệt các Item nằm trong Lô hiện tại (lot->Items)
                        // Không dùng currentCart ở đây vì currentCart chứa tất cả hàng
                        for each(ImportItem ^ item in lot->Items) {
                            std::string sSP = ToStdString(item->MaSP->Trim());
                            strcpy_s((char*)cSP, 256, sSP.c_str());
                            dSL = item->SoLuong;
                            dGia = item->GiaVon;
                            dTT = item->ThanhTien();

                            if (SQLExecute(hStmt) != SQL_SUCCESS) {
                                ShowSQLError(hStmt, SQL_HANDLE_STMT, "Chi tiết SP: " + item->MaSP);
                                throw gcnew Exception("Lỗi thêm chi tiết sản phẩm");
                            }
                        }
                        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
                    }
                } // Kết thúc vòng lặp For Each Lot

                // 4. COMMIT GIAO DỊCH
                SQLEndTran(SQL_HANDLE_DBC, hDbc, SQL_COMMIT);

                // 5. Hoàn tất UI
                ShowLoading(false);
                MessageBox::Show(L"✅ Đã tạo thành công " + count + L" đơn nhập hàng!", L"Thành công", MessageBoxButtons::OK, MessageBoxIcon::Information);

                this->DialogResult = System::Windows::Forms::DialogResult::OK;
                this->Close();
            }
            catch (Exception^ ex) {
                // Rollback nếu có lỗi
                if (hDbc != SQL_NULL_HDBC) {
                    SQLEndTran(SQL_HANDLE_DBC, hDbc, SQL_ROLLBACK);
                }
                ShowLoading(false);
                MessageBox::Show(L"❌ Lỗi xử lý: " + ex->Message, L"Thất bại", MessageBoxButtons::OK, MessageBoxIcon::Error);
            }
            finally {
                // Dọn dẹp bộ nhớ
                if (hStmt != SQL_NULL_HSTMT) SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
                if (hDbc != SQL_NULL_HDBC) {
                    SQLSetConnectAttr(hDbc, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER)SQL_AUTOCOMMIT_ON, 0);
                    SQLDisconnect(hDbc);
                    SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
                }
                if (hEnv != SQL_NULL_HENV) SQLFreeHandle(SQL_HANDLE_ENV, hEnv);

                btnConfirm->Enabled = true;
            }
        }
    };
}