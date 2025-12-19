#include "OrderConfirmationForm.h"
#include <sql.h> 
#include <sqlext.h>
#include <msclr/marshal_cppstd.h>
#include <iostream>
#include <ctime>
#include <cmath>
#include <string>

namespace PBL2QuanLyKho {

    // [CẤU HÌNH KÍCH THƯỚC NHỎ GỌN]
    const int FORM_WIDTH = 600;       // Chiều rộng Form (Đã thu nhỏ từ 900 -> 600)
    const int CONTENT_WIDTH = 540;    // Chiều rộng nội dung bên trong
    const int INNER_WIDTH = 510;      // Chiều rộng các dòng con

    // Hàm chuyển đổi số giờ (float) -> Chuỗi "X giờ Y phút"
    String^ OrderConfirmationForm::FormatTime(float totalHours) {
        if (totalHours > 10000 || totalHours < 0) {
            return L"Đang cập nhật";
        }
        if (totalHours <= 0) return L"Tức thì";
        int h = (int)totalHours;
        int m = (int)((totalHours - h) * 60);
        if (h == 0 && m > 0) return m + L" phút";
        if (m == 0 && h > 0) return h + L" giờ";
        if (h == 0 && m == 0) return L"Tức thì";
        return h + L" giờ " + m + L" phút";
    }

    // CONSTRUCTOR
    OrderConfirmationForm::OrderConfirmationForm(
        List<CartItem^>^ cartItems, String^ maSieuThi, String^ tenST, String^ diaChiST, String^ sdtST,
        Dictionary<String^, String^>^ imgMap, System::IntPtr hDbcPtr,
        double fastCost, float fastTime, double stdCost, float stdTime, String^ maYeuCau
    ) {
        _cartItems = cartItems; _maSieuThi = maSieuThi;
        _tenSieuThi = tenST; _diaChiSieuThi = diaChiST;
        _sdtSieuThi = sdtST;
        _imageMap = imgMap; _hDbc = (SQLHDBC)hDbcPtr.ToPointer();
        this->costFast = fastCost; this->costStd = stdCost;
        this->timeFast = fastTime; this->timeStd = stdTime;
        MaYeuCauGoc = maYeuCau;

        InitializeComponent();
        LoadData();
    }

    // INITIALIZE COMPONENT
    void OrderConfirmationForm::InitializeComponent()
    {
        this->Text = L"Thanh Toán";
        // [CẤU HÌNH] Kích thước form nhỏ gọn
        this->Size = System::Drawing::Size(FORM_WIDTH, 850); // 600x850
        this->StartPosition = FormStartPosition::CenterParent;
        this->BackColor = Color::White;

        // Bỏ viền Windows & Thêm Padding
        this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::None;
        this->Padding = System::Windows::Forms::Padding(2);

        this->Deactivate += gcnew EventHandler(this, &OrderConfirmationForm::OnDeactivate);
        this->Paint += gcnew PaintEventHandler(this, &OrderConfirmationForm::DrawFormBorder);
        this->MaximizeBox = false;

        // --- HEADER (ĐÃ THÊM NÚT TẮT) ---
        pnlHeader = gcnew Panel();
        pnlHeader->Dock = DockStyle::Top;
        pnlHeader->Height = 60;
        pnlHeader->BackColor = Color::White;

        // 1. Kẻ đường gạch dưới
        Panel^ lineHeader = gcnew Panel();
        lineHeader->Dock = DockStyle::Bottom;
        lineHeader->Height = 1;
        lineHeader->BackColor = Color::FromArgb(230, 230, 230);

        // 2. [THÊM MỚI] Nút Tắt (X) - Nằm góc phải
        Button^ btnClose = gcnew Button();
        btnClose->Text = L"✕";
        btnClose->Dock = DockStyle::Right; // Tự động dính sang phải
        btnClose->Width = 50;
        btnClose->FlatStyle = FlatStyle::Flat;
        btnClose->FlatAppearance->BorderSize = 0;
        btnClose->Cursor = Cursors::Hand;
        btnClose->Font = (gcnew System::Drawing::Font(L"Arial", 14, FontStyle::Regular));
        btnClose->ForeColor = Color::DimGray;
        // Hiệu ứng khi di chuột vào -> Màu đỏ
        btnClose->FlatAppearance->MouseOverBackColor = Color::FromArgb(232, 17, 35);
        btnClose->FlatAppearance->MouseDownBackColor = Color::FromArgb(200, 0, 0);
        // Sự kiện click -> Gọi hàm Back (Thoát)
        btnClose->Click += gcnew EventHandler(this, &OrderConfirmationForm::btnBack_Click);

        // 3. Tiêu đề
        Label^ lblTitle = gcnew Label();
        lblTitle->Text = L"Thanh Toán";
        lblTitle->Font = (gcnew System::Drawing::Font(L"Segoe UI", 14, FontStyle::Bold));
        lblTitle->Location = Point(20, 15);
        lblTitle->AutoSize = true;

        // Add controls vào Header (Thứ tự quan trọng để Dock hoạt động đúng)
        pnlHeader->Controls->Add(btnClose);    // Add nút X trước (để nó chiếm lề phải)
        pnlHeader->Controls->Add(lineHeader);  // Add đường kẻ (để nó nằm dưới đáy)
        pnlHeader->Controls->Add(lblTitle);    // Add tiêu đề sau cùng

        // FOOTER & SCROLL (GIỮ NGUYÊN)
        pnlFooter = CreateFixedBottomPanel();

        pnlMainScroll = gcnew FlowLayoutPanel();
        pnlMainScroll->Dock = DockStyle::Fill; pnlMainScroll->AutoScroll = true;
        pnlMainScroll->FlowDirection = FlowDirection::TopDown; pnlMainScroll->WrapContents = false;
        pnlMainScroll->Padding = System::Windows::Forms::Padding(15, 15, 15, 100);
        pnlMainScroll->BackColor = Color::FromArgb(245, 245, 250);
        pnlMainScroll->SizeChanged += gcnew EventHandler(this, &OrderConfirmationForm::OnResizeList);

        this->Controls->Add(pnlMainScroll);
        this->Controls->Add(pnlFooter);
        this->Controls->Add(pnlHeader);
        pnlFooter->BringToFront(); pnlHeader->BringToFront();
    }
    // LOAD DATA
    void OrderConfirmationForm::LoadData() {
        pnlMainScroll->Controls->Clear();
        _subTotal = 0; selectedShipOption = 0;

        // Lấy chiều rộng thực tế để tính toán (Responsive)
        int currentWidth = pnlMainScroll->ClientSize.Width - 30;
        if (currentWidth < 300) currentWidth = CONTENT_WIDTH; // Fallback

        // A. ĐỊA CHỈ
        Panel^ pnlAddress = gcnew Panel();
        pnlAddress->Width = currentWidth;
        pnlAddress->AutoSize = true; pnlAddress->BackColor = Color::Transparent;
        pnlAddress->Margin = System::Windows::Forms::Padding(0, 0, 0, 10);
        pnlAddress->Paint += gcnew PaintEventHandler(this, &OrderConfirmationForm::DrawRoundedPanel);

        int topMargin = 70;
        Label^ lblIcon = gcnew Label(); lblIcon->Text = L"📍"; lblIcon->Font = (gcnew System::Drawing::Font(L"Segoe UI Symbol", 12)); lblIcon->ForeColor = Color::FromArgb(238, 77, 45); lblIcon->Location = Point(15, topMargin); lblIcon->AutoSize = true;

        Label^ lblNamePhone = gcnew Label();
        String^ hienThiTen = String::IsNullOrEmpty(_tenSieuThi) ? _maSieuThi : _tenSieuThi;
        String^ hienThiSDT = String::IsNullOrEmpty(_sdtSieuThi) ? "(Chưa có SĐT)" : _sdtSieuThi;
        lblNamePhone->Text = hienThiTen + L"  |  " + hienThiSDT;
        lblNamePhone->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11, FontStyle::Bold)); lblNamePhone->Location = Point(45, topMargin); lblNamePhone->AutoSize = true;

        Label^ lblAddrDetail = gcnew Label(); lblAddrDetail->Text = _diaChiSieuThi; lblAddrDetail->Font = (gcnew System::Drawing::Font(L"Segoe UI", 10)); lblAddrDetail->ForeColor = Color::FromArgb(80, 80, 80);
        lblAddrDetail->Location = Point(45, topMargin + 25);
        // Giới hạn chiều rộng text để nó tự xuống dòng trong khung nhỏ
        lblAddrDetail->MaximumSize = System::Drawing::Size(currentWidth - 60, 0);
        lblAddrDetail->AutoSize = true;

        pnlAddress->Controls->Add(lblIcon); pnlAddress->Controls->Add(lblNamePhone); pnlAddress->Controls->Add(lblAddrDetail);
        Label^ dummyAddr = gcnew Label(); dummyAddr->Location = Point(0, lblAddrDetail->Bottom + 25); pnlAddress->Controls->Add(dummyAddr);
        pnlMainScroll->Controls->Add(pnlAddress);

        // B. SẢN PHẨM
        Panel^ pnlPrdContainer = gcnew Panel();
        pnlPrdContainer->Width = currentWidth;
        pnlPrdContainer->AutoSize = true; pnlPrdContainer->BackColor = Color::Transparent; pnlPrdContainer->Margin = System::Windows::Forms::Padding(0, 0, 0, 15);
        pnlPrdContainer->Paint += gcnew PaintEventHandler(this, &OrderConfirmationForm::DrawRoundedPanel);
        Label^ lblPrdTitle = gcnew Label(); lblPrdTitle->Text = L"Danh Sách Sản phẩm"; lblPrdTitle->Font = (gcnew System::Drawing::Font(L"Segoe UI", 10, FontStyle::Bold)); lblPrdTitle->Location = Point(15, 15); lblPrdTitle->AutoSize = true; pnlPrdContainer->Controls->Add(lblPrdTitle);

        int yPos = 45;
        for each (CartItem ^ item in _cartItems) {
            Panel^ row = gcnew Panel();
            row->Size = System::Drawing::Size(currentWidth - 30, 80);
            row->Location = Point(15, yPos); row->BackColor = Color::Transparent;

            PictureBox^ pic = gcnew PictureBox(); pic->Size = System::Drawing::Size(60, 60); pic->Location = Point(0, 5); pic->SizeMode = PictureBoxSizeMode::Zoom; pic->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
            if (_imageMap != nullptr && _imageMap->ContainsKey(item->MaSP)) { try { pic->ImageLocation = _imageMap[item->MaSP]; } catch (...) {} }
            row->Controls->Add(pic);

            Label^ lName = gcnew Label(); lName->Text = item->TenSP; lName->Font = (gcnew System::Drawing::Font(L"Segoe UI", 10)); lName->Location = Point(70, 5);
            lName->MaximumSize = System::Drawing::Size(currentWidth - 180, 40); // Thu hẹp tên SP lại
            lName->AutoSize = true; row->Controls->Add(lName);

            Label^ lQty = gcnew Label(); lQty->Text = L"x" + item->SoLuong; lQty->ForeColor = Color::Gray; lQty->Location = Point(70, 45); lQty->AutoSize = true; row->Controls->Add(lQty);

            Label^ lPrice = gcnew Label(); lPrice->Text = item->ThanhTien.ToString("N0") + L" ₫"; lPrice->Font = (gcnew System::Drawing::Font(L"Segoe UI", 10, FontStyle::Bold));
            // Căn phải giá tiền
            lPrice->AutoSize = true;
            lPrice->Anchor = static_cast<AnchorStyles>(AnchorStyles::Top | AnchorStyles::Right);
            lPrice->Location = Point(row->Width - 100, 25); // Vị trí tạm
            lPrice->TextAlign = ContentAlignment::MiddleRight;

            // Hack căn phải thủ công một chút
            int wPrice = TextRenderer::MeasureText(lPrice->Text, lPrice->Font).Width;
            lPrice->Location = Point(row->Width - wPrice - 5, 25);

            row->Controls->Add(lPrice);

            pnlPrdContainer->Controls->Add(row); yPos += 75; _subTotal += item->ThanhTien;
        }
        Label^ dummyPrd = gcnew Label(); dummyPrd->Location = Point(0, yPos + 10); pnlPrdContainer->Controls->Add(dummyPrd);
        pnlMainScroll->Controls->Add(pnlPrdContainer);

        // C. VẬN CHUYỂN
        Panel^ pnlShipContainer = gcnew Panel();
        pnlShipContainer->Width = currentWidth;
        pnlShipContainer->AutoSize = true; pnlShipContainer->BackColor = Color::Transparent; pnlShipContainer->Margin = System::Windows::Forms::Padding(0, 0, 0, 15);
        pnlShipContainer->Paint += gcnew PaintEventHandler(this, &OrderConfirmationForm::DrawRoundedPanel);
        Label^ lblShipTitle = gcnew Label(); lblShipTitle->Text = L"Phương Thức Vận Chuyển"; lblShipTitle->Font = (gcnew System::Drawing::Font(L"Segoe UI", 10, FontStyle::Bold)); lblShipTitle->Location = Point(15, 15); lblShipTitle->AutoSize = true; pnlShipContainer->Controls->Add(lblShipTitle);

        pnlShipFast = CreateShippingOptionPanel(L"Nhanh", L"Dự kiến: " + FormatTime(timeFast), costFast, 1);
        pnlShipFast->Location = Point(15, 50);

        pnlShipStandard = CreateShippingOptionPanel(L"Tiêu Chuẩn", L"Dự kiến: " + FormatTime(timeStd), costStd, 2);
        pnlShipStandard->Location = Point(15, 140);

        pnlShipContainer->Controls->Add(pnlShipFast); pnlShipContainer->Controls->Add(pnlShipStandard);
        Label^ dummyShip = gcnew Label(); dummyShip->Location = Point(0, 235); pnlShipContainer->Controls->Add(dummyShip);
        pnlMainScroll->Controls->Add(pnlShipContainer);

        // D. CHI TIẾT THANH TOÁN
        Panel^ pnlPayment = CreatePaymentDetailBox();
        pnlMainScroll->Controls->Add(pnlPayment);

        Panel^ spacer = gcnew Panel(); spacer->Height = 120; spacer->Width = 10; spacer->BackColor = Color::Transparent; pnlMainScroll->Controls->Add(spacer);

        if (costFast > 1) OnShipOptionClick(pnlShipFast, EventArgs::Empty);
        else OnShipOptionClick(pnlShipStandard, EventArgs::Empty);
    }

    // [FIXED] FOOTER DÙNG DOCKING ĐỂ KHÔNG BỊ LỆCH
    Panel^ OrderConfirmationForm::CreateFixedBottomPanel() {
        Panel^ p = gcnew Panel();
        p->Dock = DockStyle::Bottom;
        p->Height = 80;
        p->BackColor = Color::White;
        p->Paint += gcnew PaintEventHandler(this, &OrderConfirmationForm::DrawTopBorder);

        btnConfirm = gcnew Button();
        btnConfirm->Text = L"Đặt Hàng";
        btnConfirm->BackColor = Color::FromArgb(238, 77, 45);
        btnConfirm->ForeColor = Color::White;
        btnConfirm->Font = (gcnew System::Drawing::Font(L"Segoe UI", 12, FontStyle::Bold));
        btnConfirm->FlatStyle = FlatStyle::Flat;
        btnConfirm->FlatAppearance->BorderSize = 0;
        btnConfirm->Size = System::Drawing::Size(160, 80); // Nút nhỏ hơn xíu cho hợp form nhỏ
        btnConfirm->Dock = DockStyle::Right;
        btnConfirm->Cursor = Cursors::Hand;
        btnConfirm->Click += gcnew EventHandler(this, &OrderConfirmationForm::btnConfirm_Click);

        Panel^ pnlText = gcnew Panel();
        pnlText->Dock = DockStyle::Fill;

        Panel^ pnlPriceGroup = gcnew Panel();
        pnlPriceGroup->Width = 300;
        pnlPriceGroup->Dock = DockStyle::Right;
        pnlPriceGroup->Padding = System::Windows::Forms::Padding(0, 10, 15, 10);

        Label^ lTotalTitle = gcnew Label();
        lTotalTitle->Text = L"Tổng thanh toán";
        lTotalTitle->Font = (gcnew System::Drawing::Font(L"Segoe UI", 10));
        lTotalTitle->ForeColor = Color::FromArgb(80, 80, 80);
        lTotalTitle->AutoSize = false;
        lTotalTitle->Size = System::Drawing::Size(285, 25);
        lTotalTitle->TextAlign = ContentAlignment::BottomRight;
        lTotalTitle->Dock = DockStyle::Top;

        lblTotalFooter = gcnew Label();
        lblTotalFooter->Text = L"0 ₫";
        lblTotalFooter->Font = (gcnew System::Drawing::Font(L"Segoe UI", 16, FontStyle::Bold));
        lblTotalFooter->ForeColor = Color::FromArgb(238, 77, 45);
        lblTotalFooter->AutoSize = false;
        lblTotalFooter->Size = System::Drawing::Size(285, 40);
        lblTotalFooter->TextAlign = ContentAlignment::TopRight;
        lblTotalFooter->Dock = DockStyle::Top;

        pnlPriceGroup->Controls->Add(lblTotalFooter);
        pnlPriceGroup->Controls->Add(lTotalTitle);
        pnlText->Controls->Add(pnlPriceGroup);

        p->Controls->Add(pnlText);
        p->Controls->Add(btnConfirm);
        return p;
    }

    void OrderConfirmationForm::DrawShippingPanel(Object^ sender, PaintEventArgs^ e) {
        Panel^ p = (Panel^)sender; Graphics^ g = e->Graphics;
        g->SmoothingMode = SmoothingMode::AntiAlias; g->PixelOffsetMode = PixelOffsetMode::HighQuality;

        int currentId = (int)p->Tag;
        bool isSelected = (currentId == selectedShipOption);
        Color borderColor = isSelected ? Color::FromArgb(238, 77, 45) : Color::FromArgb(180, 180, 180);
        Color backColor = isSelected ? Color::FromArgb(254, 245, 240) : Color::White;

        System::Drawing::Rectangle rect = p->ClientRectangle; rect.Width -= 1; rect.Height -= 1; int radius = 8;
        GraphicsPath^ path = gcnew GraphicsPath();
        path->AddArc(rect.X, rect.Y, radius, radius, 180, 90); path->AddArc(rect.Right - radius, rect.Y, radius, radius, 270, 90);
        path->AddArc(rect.Right - radius, rect.Bottom - radius, radius, radius, 0, 90); path->AddArc(rect.X, rect.Bottom - radius, radius, radius, 90, 90); path->CloseFigure();

        g->FillPath(gcnew SolidBrush(backColor), path);
        g->DrawPath(gcnew Pen(borderColor, 1), path);

        if (isSelected) {
            cli::array<Point>^ points = gcnew cli::array<Point>(3) { Point(rect.Right, rect.Top), Point(rect.Right, rect.Top + 20), Point(rect.Right - 20, rect.Top) };
            g->FillPolygon(gcnew SolidBrush(borderColor), points);
            g->DrawLine(gcnew Pen(Color::White, 1.5f), rect.Right - 12, rect.Top + 6, rect.Right - 8, rect.Top + 10);
            g->DrawLine(gcnew Pen(Color::White, 1.5f), rect.Right - 8, rect.Top + 10, rect.Right - 4, rect.Top + 4);
        }
    }

    Panel^ OrderConfirmationForm::CreateShippingOptionPanel(String^ name, String^ time, double cost, int id) {
        // Tính chiều rộng động
        int w = pnlMainScroll->ClientSize.Width - 60;
        if (w < 300) w = 500;

        Panel^ p = gcnew Panel();
        p->Size = System::Drawing::Size(w, 80);
        p->BackColor = Color::Transparent; p->Cursor = Cursors::Hand; p->Tag = id;
        p->Paint += gcnew PaintEventHandler(this, &OrderConfirmationForm::DrawShippingPanel);
        p->Click += gcnew EventHandler(this, &OrderConfirmationForm::OnShipOptionClick);

        Label^ lName = gcnew Label(); lName->Text = name; lName->Font = (gcnew System::Drawing::Font(L"Segoe UI", 10, FontStyle::Bold)); lName->Location = Point(15, 15); lName->AutoSize = true; lName->Tag = id; lName->Click += gcnew EventHandler(this, &OrderConfirmationForm::OnShipOptionClick);
        Label^ lTime = gcnew Label(); lTime->Text = time; lTime->ForeColor = Color::Gray; lTime->Location = Point(15, 40); lTime->AutoSize = true; lTime->Tag = id; lTime->Click += gcnew EventHandler(this, &OrderConfirmationForm::OnShipOptionClick);

        Label^ lPrice = gcnew Label(); lPrice->Text = (cost < 1) ? L"Không hỗ trợ" : (cost.ToString("N0") + L" ₫");
        lPrice->Font = (gcnew System::Drawing::Font(L"Segoe UI", 10, FontStyle::Bold)); lPrice->AutoSize = true; lPrice->Tag = id; lPrice->Click += gcnew EventHandler(this, &OrderConfirmationForm::OnShipOptionClick);

        // Căn phải giá
        int textW = TextRenderer::MeasureText(lPrice->Text, lPrice->Font).Width;
        lPrice->Location = Point(w - textW - 20, 30);

        p->Controls->Add(lName); p->Controls->Add(lTime); p->Controls->Add(lPrice);
        return p;
    }

    // UI HELPERS
    Label^ OrderConfirmationForm::CreateSectionTitle(String^ text) { Label^ lbl = gcnew Label(); lbl->Text = text; lbl->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11, FontStyle::Bold)); lbl->Location = Point(15, 15); lbl->AutoSize = true; return lbl; }

    Panel^ OrderConfirmationForm::CreateSectionPanel(int height) {
        // Responsive width
        int w = pnlMainScroll->ClientSize.Width - 30;
        if (w < 300) w = CONTENT_WIDTH;

        Panel^ p = gcnew Panel(); p->BackColor = Color::White;
        p->Width = w;
        if (height > 0) p->Height = height; p->Margin = System::Windows::Forms::Padding(0, 0, 0, 15); p->Padding = System::Windows::Forms::Padding(10); p->Paint += gcnew PaintEventHandler(this, &OrderConfirmationForm::DrawRoundedPanel); return p;
    }

    void OrderConfirmationForm::DrawRoundedPanel(Object^ sender, PaintEventArgs^ e) { Panel^ p = (Panel^)sender; Graphics^ g = e->Graphics; g->SmoothingMode = SmoothingMode::AntiAlias; g->PixelOffsetMode = PixelOffsetMode::HighQuality; System::Drawing::Rectangle rect = p->ClientRectangle; rect.Width -= 1; rect.Height -= 1; int radius = 10; GraphicsPath^ path = gcnew GraphicsPath(); path->AddArc(rect.X, rect.Y, radius, radius, 180, 90); path->AddArc(rect.Right - radius, rect.Y, radius, radius, 270, 90); path->AddArc(rect.Right - radius, rect.Bottom - radius, radius, radius, 0, 90); path->AddArc(rect.X, rect.Bottom - radius, radius, radius, 90, 90); path->CloseFigure(); g->FillPath(Brushes::White, path); g->DrawPath(gcnew Pen(Color::FromArgb(180, 180, 180), 1), path); }

    Panel^ OrderConfirmationForm::CreatePaymentDetailBox() {
        Panel^ p = CreateSectionPanel(180);
        int w = p->Width;

        Label^ lblTitle = gcnew Label(); lblTitle->Text = L"Chi tiết thanh toán"; lblTitle->Font = (gcnew System::Drawing::Font(L"Segoe UI", 12, FontStyle::Bold)); lblTitle->Location = Point(15, 15); lblTitle->AutoSize = true;

        Label^ l1 = gcnew Label(); l1->Text = L"Tổng tiền hàng"; l1->Location = Point(15, 55); l1->AutoSize = true; l1->ForeColor = Color::FromArgb(80, 80, 80);
        lblDetailSubTotal = gcnew Label(); lblDetailSubTotal->Text = L"0 ₫"; lblDetailSubTotal->AutoSize = true;
        lblDetailSubTotal->Location = Point(w - 100, 55);
        lblDetailSubTotal->Anchor = static_cast<AnchorStyles>(AnchorStyles::Top | AnchorStyles::Right);

        Label^ l2 = gcnew Label(); l2->Text = L"Tổng tiền phí vận chuyển"; l2->Location = Point(15, 85); l2->AutoSize = true; l2->ForeColor = Color::FromArgb(80, 80, 80);
        lblDetailShip = gcnew Label(); lblDetailShip->Text = L"0 ₫"; lblDetailShip->AutoSize = true;
        lblDetailShip->Location = Point(w - 100, 85);
        lblDetailShip->TextAlign = ContentAlignment::MiddleRight;

        Panel^ line = gcnew Panel();
        line->Size = System::Drawing::Size(w - 30, 1);
        line->BackColor = Color::FromArgb(230, 230, 230); line->Location = Point(15, 120);

        Label^ l3 = gcnew Label(); l3->Text = L"Tổng thanh toán"; l3->Location = Point(15, 135); l3->AutoSize = true; l3->Font = (gcnew System::Drawing::Font(L"Segoe UI", 13, FontStyle::Bold));

        lblDetailFinal = gcnew Label(); lblDetailFinal->Text = L"0 ₫";
        lblDetailFinal->Location = Point(w - 180, 135);
        lblDetailFinal->Size = System::Drawing::Size(150, 30); lblDetailFinal->TextAlign = ContentAlignment::MiddleRight; lblDetailFinal->ForeColor = Color::FromArgb(238, 77, 45); lblDetailFinal->Font = (gcnew System::Drawing::Font(L"Segoe UI", 13, FontStyle::Bold));

        p->Controls->Add(lblTitle); p->Controls->Add(l1); p->Controls->Add(lblDetailSubTotal); p->Controls->Add(l2); p->Controls->Add(lblDetailShip); p->Controls->Add(line); p->Controls->Add(l3); p->Controls->Add(lblDetailFinal);
        return p;
    }

    void OrderConfirmationForm::OnShipOptionClick(Object^ sender, EventArgs^ e) {
        Control^ c = (Control^)sender; int id = (int)c->Tag; selectedShipOption = id;
        if (id == 1) { SelectedShippingMethod = "GiaoNhanh"; FinalShippingCost = costFast; }
        else if (id == 2) { SelectedShippingMethod = "GiaoThuong"; FinalShippingCost = costStd; }
        pnlShipFast->Invalidate(); pnlShipStandard->Invalidate();
        UpdateTotalUI();
    }

    void OrderConfirmationForm::UpdateTotalUI() {
        FinalTotal = _subTotal + FinalShippingCost;
        String^ strTotal = FinalTotal.ToString("N0") + L" ₫"; String^ strShip = FinalShippingCost.ToString("N0") + L" ₫"; String^ strSub = _subTotal.ToString("N0") + L" ₫";

        if (lblTotalFooter) lblTotalFooter->Text = strTotal;

        // Căn chỉnh bảng chi tiết (responsive theo khổ giấy)
        int w = 500; // Giá trị ước lượng, thực tế sẽ lấy từ parent
        if (lblDetailSubTotal && lblDetailSubTotal->Parent) w = lblDetailSubTotal->Parent->Width;

        if (lblDetailSubTotal) {
            lblDetailSubTotal->Text = strSub;
            int textW = TextRenderer::MeasureText(strSub, lblDetailSubTotal->Font).Width;
            lblDetailSubTotal->Location = Point(w - textW - 30, lblDetailSubTotal->Location.Y);
        }
        if (lblDetailShip) {
            lblDetailShip->Text = strShip;
            int textW = TextRenderer::MeasureText(strShip, lblDetailShip->Font).Width;
            lblDetailShip->Location = Point(w - textW - 30, lblDetailShip->Location.Y);
        }
        if (lblDetailFinal) lblDetailFinal->Text = strTotal;
    }

    void OrderConfirmationForm::btnConfirm_Click(Object^ sender, EventArgs^ e) {
        if (selectedShipOption == 0) { MessageBox::Show(L"Vui lòng chọn phương thức vận chuyển!", L"Thông báo", MessageBoxButtons::OK, MessageBoxIcon::Warning); return; }
        IsConfirmed = true; this->DialogResult = System::Windows::Forms::DialogResult::OK; this->Close();
    }
    void OrderConfirmationForm::btnBack_Click(Object^ sender, EventArgs^ e) { this->DialogResult = System::Windows::Forms::DialogResult::Cancel; this->Close(); }
    void OrderConfirmationForm::DrawTopBorder(Object^ sender, PaintEventArgs^ e) { Panel^ p = (Panel^)sender; e->Graphics->DrawLine(gcnew Pen(Color::LightGray, 1), 0, 0, p->Width, 0); }

    // Resize động
    void OrderConfirmationForm::OnResizeList(Object^ sender, EventArgs^ e) {
        int w = pnlMainScroll->ClientSize.Width - 30;
        for each (Control ^ c in pnlMainScroll->Controls) {
            if (dynamic_cast<Panel^>(c)) {
                c->Width = w;
                // Nếu là panel chi tiết thanh toán, chỉnh lại vị trí label bên trong
                if (c->Controls->Contains(lblDetailFinal)) {
                    lblDetailFinal->Location = Point(w - 180, lblDetailFinal->Location.Y);
                    if (lblDetailSubTotal) lblDetailSubTotal->Location = Point(w - TextRenderer::MeasureText(lblDetailSubTotal->Text, lblDetailSubTotal->Font).Width - 30, lblDetailSubTotal->Location.Y);
                    if (lblDetailShip) lblDetailShip->Location = Point(w - TextRenderer::MeasureText(lblDetailShip->Text, lblDetailShip->Font).Width - 30, lblDetailShip->Location.Y);
                }
            }
        }
    }
    void OrderConfirmationForm::DrawDecoLine(Object^ sender, PaintEventArgs^ e) {}

    void OrderConfirmationForm::OnDeactivate(Object^ sender, EventArgs^ e) {
        if (!IsConfirmed) {
            this->DialogResult = System::Windows::Forms::DialogResult::Cancel;
            this->Close();
        }
    }

    void OrderConfirmationForm::DrawFormBorder(Object^ sender, PaintEventArgs^ e) {
        Control^ c = (Control^)sender;
        Pen^ p = gcnew Pen(Color::DimGray, 4);
        System::Drawing::Rectangle rect = c->ClientRectangle;
        e->Graphics->DrawRectangle(p, rect);
    }
}