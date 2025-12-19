#include "BaiToanKho.h"
#include <iostream>
#include <iomanip>
#include <algorithm> // Cần cho std::reverse

using namespace std;

// Hàm so sánh để sắp xếp ưu tiên: Cấp bách trước -> Giá trị cao trước
bool SoSanhDoUuTien(const LoHangNhap& a, const LoHangNhap& b) {
    if (a.laHangCapBach != b.laHangCapBach) {
        return a.laHangCapBach > b.laHangCapBach; // True (1) > False (0) -> Cấp bách lên đầu
    }
    // Nếu cùng độ cấp bách, ưu tiên lô có giá trị kinh tế cao hơn
    return a.giaTriLo > b.giaTriLo;
}

// Hàm Giải bài toán Balo (Knapsack) cho 1 kho cụ thể
std::vector<LoHangNhap> QuanLySucChuaKho::GiaiBaiToanBaloChoMotKho(float sucChuaKho, std::vector<LoHangNhap>& dsUngVien) {
    int n = dsUngVien.size();
    int W = (int)sucChuaKho; 
    
    // Bảng DP: dp[w] = Giá trị lớn nhất tại dung tích w
    std::vector<float> dp(W + 1, 0.0f);
    
    // Mảng truy vết: keep[i][w] = true nếu vật thứ i được chọn tại dung tích w
    std::vector<std::vector<bool>> keep(n, std::vector<bool>(W + 1, false));

    for (int i = 0; i < n; ++i) {
        int wi = (int)dsUngVien[i].theTichLo; // Thể tích vật i
        float vi = dsUngVien[i].giaTriLo;     // Giá trị vật i

        for (int w = W; w >= wi; --w) {
            if (dp[w - wi] + vi > dp[w]) {
                dp[w] = dp[w - wi] + vi;
                keep[i][w] = true;
            }
        }
    }

    // Truy vết để lấy danh sách các món được chọn
    std::vector<LoHangNhap> duocChon;
    std::vector<LoHangNhap> conLai;
    
    int wHienTai = W;
    for (int i = n - 1; i >= 0; --i) {
        int wi = (int)dsUngVien[i].theTichLo;
        // Kiểm tra xem vật i có được chọn ở trạng thái dung tích wHienTai không
        if (keep[i][wHienTai]) {
            duocChon.push_back(dsUngVien[i]);
            wHienTai -= wi;
        } else {
            conLai.push_back(dsUngVien[i]); // Món này không được chọn, giữ lại cho kho sau
        }
    }

    // Cập nhật lại danh sách ứng viên (chỉ còn những món chưa được chọn để trả về cho hàm gọi)
    // Đảo ngược lại conLai để giữ thứ tự ưu tiên ban đầu (do truy vết đi ngược từ n về 0)
    std::reverse(conLai.begin(), conLai.end());
    dsUngVien = conLai; 

    return duocChon;
}

std::vector<KetQuaKho> QuanLySucChuaKho::ToiUuNhapHangVaoKho(std::vector<Kho> dsKhoSanCo, std::vector<LoHangNhap> dsHangCanNhap) {
    std::vector<KetQuaKho> ketQuaChung;

    // B1: Sắp xếp hàng hóa theo độ ưu tiên (Cấp bách -> Giá trị)
    std::sort(dsHangCanNhap.begin(), dsHangCanNhap.end(), SoSanhDoUuTien);

    // B2: Duyệt qua từng kho và lấp đầy
    for (const auto& kho : dsKhoSanCo) {
        if (dsHangCanNhap.empty()) break; // Đã xếp hết hàng

        KetQuaKho kq;
        kq.kho = kho;
        
        // Gọi thuật toán Balo để chọn hàng tốt nhất cho kho này
        // Lưu ý: Hàm này sẽ TỰ ĐỘNG LOẠI BỎ các món đã chọn ra khỏi dsHangCanNhap
        kq.dsLoHangDuocChon = GiaiBaiToanBaloChoMotKho(kho.getSucChua(), dsHangCanNhap); 
        
        // Tính tổng kết quả
        for (const auto& hang : kq.dsLoHangDuocChon) {
            kq.tongTheTichSuDung += hang.theTichLo;
            kq.tongGiaTri += hang.giaTriLo;
        }

        if (!kq.dsLoHangDuocChon.empty()) {
            ketQuaChung.push_back(kq);
        }
    }

    // B3: Kiểm tra hàng còn dư (Không kho nào chứa nổi)
    if (!dsHangCanNhap.empty()) {
        cout << "\n!! CANH BAO: Con " << dsHangCanNhap.size() << " lo hang chua duoc xep vao kho (Het dung tich) !!\n";
        for(auto& h : dsHangCanNhap) {
            cout << "   - " << h.tenSP << " (" << (h.laHangCapBach ? "CAP BACH" : "Thuong") << ")\n";
        }
    }

    return ketQuaChung;
}