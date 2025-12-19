#pragma once
#include <vector>
#include <string>

using namespace System;
using namespace System::Collections::Generic;

namespace PBL2QuanLyKho {
	public ref class VietnamData {
	public:
		// Hàm trả về Dictionary chứa toàn bộ dữ liệu 63 Tỉnh Thành
		static Dictionary<String^, List<String^>^>^ GetFullData() {
			Dictionary<String^, List<String^>^>^ data = gcnew Dictionary<String^, List<String^>^>();

			// =================================================================================
			// 1. VÙNG ĐÔNG BẮC BỘ & TÂY BẮC BỘ
			// =================================================================================
			data->Add(L"Hà Giang", TaoList(gcnew cli::array<String^>{L"TP. Hà Giang", L"Đồng Văn", L"Mèo Vạc", L"Yên Minh", L"Quản Bạ", L"Vị Xuyên", L"Bắc Mê", L"Hoàng Su Phì", L"Xín Mần", L"Bắc Quang", L"Quang Bình"}));
			data->Add(L"Cao Bằng", TaoList(gcnew cli::array<String^>{L"TP. Cao Bằng", L"Bảo Lạc", L"Bảo Lâm", L"Hạ Lang", L"Hà Quảng", L"Hòa An", L"Nguyên Bình", L"Quảng Hòa", L"Thạch An", L"Trùng Khánh"}));
			data->Add(L"Lào Cai", TaoList(gcnew cli::array<String^>{L"TP. Lào Cai", L"TX. Sa Pa", L"Bát Xát", L"Bảo Thắng", L"Bảo Yên", L"Bắc Hà", L"Mường Khương", L"Si Ma Cai", L"Văn Bàn"}));
			data->Add(L"Bắc Kạn", TaoList(gcnew cli::array<String^>{L"TP. Bắc Kạn", L"Ba Bể", L"Bạch Thông", L"Chợ Đồn", L"Chợ Mới", L"Na Rì", L"Ngân Sơn", L"Pác Nặm"}));
			data->Add(L"Lạng Sơn", TaoList(gcnew cli::array<String^>{L"TP. Lạng Sơn", L"Bắc Sơn", L"Bình Gia", L"Cao Lộc", L"Chi Lăng", L"Đình Lập", L"Hữu Lũng", L"Lộc Bình", L"Tràng Định", L"Văn Lãng", L"Văn Quan"}));
			data->Add(L"Tuyên Quang", TaoList(gcnew cli::array<String^>{L"TP. Tuyên Quang", L"Chiêm Hóa", L"Hàm Yên", L"Lâm Bình", L"Na Hang", L"Sơn Dương", L"Yên Sơn"}));
			data->Add(L"Yên Bái", TaoList(gcnew cli::array<String^>{L"TP. Yên Bái", L"TX. Nghĩa Lộ", L"Lục Yên", L"Mù Cang Chải", L"Trạm Tấu", L"Trấn Yên", L"Văn Chấn", L"Văn Yên", L"Yên Bình"}));
			data->Add(L"Thái Nguyên", TaoList(gcnew cli::array<String^>{L"TP. Thái Nguyên", L"TP. Sông Công", L"TP. Phổ Yên", L"Đại Từ", L"Định Hóa", L"Đồng Hỷ", L"Phú Bình", L"Phú Lương", L"Võ Nhai"}));
			data->Add(L"Phú Thọ", TaoList(gcnew cli::array<String^>{L"TP. Việt Trì", L"TX. Phú Thọ", L"Cẩm Khê", L"Đoan Hùng", L"Hạ Hòa", L"Lâm Thao", L"Phù Ninh", L"Tam Nông", L"Tân Sơn", L"Thanh Ba", L"Thanh Sơn", L"Thanh Thủy", L"Yên Lập"}));
			data->Add(L"Bắc Giang", TaoList(gcnew cli::array<String^>{L"TP. Bắc Giang", L"TX. Việt Yên", L"Hiệp Hòa", L"Lạng Giang", L"Lục Nam", L"Lục Ngạn", L"Sơn Động", L"Tân Yên", L"Yên Dũng", L"Yên Thế"}));
			data->Add(L"Quảng Ninh", TaoList(gcnew cli::array<String^>{L"TP. Hạ Long", L"TP. Móng Cái", L"TP. Cẩm Phả", L"TP. Uông Bí", L"TX. Quảng Yên", L"TX. Đông Triều", L"Ba Chẽ", L"Bình Liêu", L"Cô Tô", L"Đầm Hà", L"Hải Hà", L"Tiên Yên", L"Vân Đồn"}));
			data->Add(L"Lai Châu", TaoList(gcnew cli::array<String^>{L"TP. Lai Châu", L"Mường Tè", L"Nậm Nhùn", L"Phong Thổ", L"Sìn Hồ", L"Tam Đường", L"Tân Uyên", L"Than Uyên"}));
			data->Add(L"Điện Biên", TaoList(gcnew cli::array<String^>{L"TP. Điện Biên Phủ", L"TX. Mường Lay", L"Điện Biên", L"Điện Biên Đông", L"Mường Ảng", L"Mường Chà", L"Mường Nhé", L"Nậm Pồ", L"Tủa Chùa", L"Tuần Giáo"}));
			data->Add(L"Sơn La", TaoList(gcnew cli::array<String^>{L"TP. Sơn La", L"Bắc Yên", L"Mai Sơn", L"Mộc Châu", L"Mường La", L"Phù Yên", L"Quỳnh Nhai", L"Sông Mã", L"Sốp Cộp", L"Thuận Châu", L"Vân Hồ", L"Yên Châu"}));
			data->Add(L"Hòa Bình", TaoList(gcnew cli::array<String^>{L"TP. Hòa Bình", L"Cao Phong", L"Đà Bắc", L"Kim Bôi", L"Lạc Sơn", L"Lạc Thủy", L"Lương Sơn", L"Mai Châu", L"Tân Lạc", L"Yên Thủy"}));

			// =================================================================================
			// 2. ĐỒNG BẰNG SÔNG HỒNG
			// =================================================================================
			data->Add(L"Hà Nội", TaoList(gcnew cli::array<String^>{L"Ba Đình", L"Hoàn Kiếm", L"Tây Hồ", L"Long Biên", L"Cầu Giấy", L"Đống Đa", L"Hai Bà Trưng", L"Hoàng Mai", L"Thanh Xuân", L"Sóc Sơn", L"Đông Anh", L"Gia Lâm", L"Nam Từ Liêm", L"Bắc Từ Liêm", L"Thanh Trì", L"Bắc Từ Liêm", L"Mê Linh", L"Hà Đông", L"Sơn Tây", L"Ba Vì", L"Phúc Thọ", L"Đan Phượng", L"Hoài Đức", L"Quốc Oai", L"Thạch Thất", L"Chương Mỹ", L"Thanh Oai", L"Thường Tín", L"Phú Xuyên", L"Ứng Hòa", L"Mỹ Đức"}));
			data->Add(L"Vĩnh Phúc", TaoList(gcnew cli::array<String^>{L"TP. Vĩnh Yên", L"TP. Phúc Yên", L"Bình Xuyên", L"Lập Thạch", L"Sông Lô", L"Tam Đảo", L"Tam Dương", L"Vĩnh Tường", L"Yên Lạc"}));
			data->Add(L"Bắc Ninh", TaoList(gcnew cli::array<String^>{L"TP. Bắc Ninh", L"TP. Từ Sơn", L"TX. Thuận Thành", L"TX. Quế Võ", L"Gia Bình", L"Lương Tài", L"Tiên Du", L"Yên Phong"}));
			data->Add(L"Hải Dương", TaoList(gcnew cli::array<String^>{L"TP. Hải Dương", L"TP. Chí Linh", L"TX. Kinh Môn", L"Bình Giang", L"Cẩm Giàng", L"Gia Lộc", L"Kim Thành", L"Nam Sách", L"Ninh Giang", L"Thanh Hà", L"Thanh Miện", L"Tứ Kỳ"}));
			data->Add(L"Hải Phòng", TaoList(gcnew cli::array<String^>{L"Hồng Bàng", L"Ngô Quyền", L"Lê Chân", L"Hải An", L"Kiến An", L"Đồ Sơn", L"Dương Kinh", L"Thủy Nguyên", L"An Dương", L"An Lão", L"Kiến Thụy", L"Tiên Lãng", L"Vĩnh Bảo", L"Cát Hải", L"Bạch Long Vĩ"}));
			data->Add(L"Hưng Yên", TaoList(gcnew cli::array<String^>{L"TP. Hưng Yên", L"TX. Mỹ Hào", L"Ân Thi", L"Khoái Châu", L"Kim Động", L"Phù Cừ", L"Tiên Lữ", L"Văn Giang", L"Văn Lâm", L"Yên Mỹ"}));
			data->Add(L"Thái Bình", TaoList(gcnew cli::array<String^>{L"TP. Thái Bình", L"Đông Hưng", L"Hưng Hà", L"Kiến Xương", L"Quỳnh Phụ", L"Thái Thụy", L"Tiền Hải", L"Vũ Thư"}));
			data->Add(L"Hà Nam", TaoList(gcnew cli::array<String^>{L"TP. Phủ Lý", L"TX. Duy Tiên", L"Bình Lục", L"Kim Bảng", L"Lý Nhân", L"Thanh Liêm"}));
			data->Add(L"Nam Định", TaoList(gcnew cli::array<String^>{L"TP. Nam Định", L"Giao Thủy", L"Hải Hậu", L"Mỹ Lộc", L"Nam Trực", L"Nghĩa Hưng", L"Trực Ninh", L"Vụ Bản", L"Xuân Trường", L"Ý Yên"}));
			data->Add(L"Ninh Bình", TaoList(gcnew cli::array<String^>{L"TP. Ninh Bình", L"TP. Tam Điệp", L"Gia Viễn", L"Hoa Lư", L"Kim Sơn", L"Nho Quan", L"Yên Khánh", L"Yên Mô"}));

			// =================================================================================
			// 3. BẮC TRUNG BỘ
			// =================================================================================
			data->Add(L"Thanh Hóa", TaoList(gcnew cli::array<String^>{L"TP. Thanh Hóa", L"TP. Sầm Sơn", L"TX. Bỉm Sơn", L"TX. Nghi Sơn", L"Bá Thước", L"Cẩm Thủy", L"Đông Sơn", L"Hà Trung", L"Hậu Lộc", L"Hoằng Hóa", L"Lang Chánh", L"Mường Lát", L"Nga Sơn", L"Ngọc Lặc", L"Như Thanh", L"Như Xuân", L"Nông Cống", L"Quan Hóa", L"Quan Sơn", L"Quảng Xương", L"Thạch Thành", L"Thiệu Hóa", L"Thọ Xuân", L"Thường Xuân", L"Triệu Sơn", L"Vĩnh Lộc", L"Yên Định"}));
			data->Add(L"Nghệ An", TaoList(gcnew cli::array<String^>{L"TP. Vinh", L"TX. Cửa Lò", L"TX. Hoàng Mai", L"TX. Thái Hòa", L"Anh Sơn", L"Con Cuông", L"Diễn Châu", L"Đô Lương", L"Hưng Nguyên", L"Kỳ Sơn", L"Nam Đàn", L"Nghi Lộc", L"Nghĩa Đàn", L"Quế Phong", L"Quỳ Châu", L"Quỳ Hợp", L"Quỳnh Lưu", L"Tân Kỳ", L"Thanh Chương", L"Tương Dương", L"Yên Thành"}));
			data->Add(L"Hà Tĩnh", TaoList(gcnew cli::array<String^>{L"TP. Hà Tĩnh", L"TX. Hồng Lĩnh", L"TX. Kỳ Anh", L"Cẩm Xuyên", L"Can Lộc", L"Đức Thọ", L"Hương Khê", L"Hương Sơn", L"Kỳ Anh", L"Lộc Hà", L"Nghi Xuân", L"Thạch Hà", L"Vũ Quang"}));
			data->Add(L"Quảng Bình", TaoList(gcnew cli::array<String^>{L"TP. Đồng Hới", L"TX. Ba Đồn", L"Bố Trạch", L"Lệ Thủy", L"Minh Hóa", L"Quảng Ninh", L"Quảng Trạch", L"Tuyên Hóa"}));
			data->Add(L"Quảng Trị", TaoList(gcnew cli::array<String^>{L"TP. Đông Hà", L"TX. Quảng Trị", L"Cam Lộ", L"Cồn Cỏ", L"Đa Krông", L"Gio Linh", L"Hải Lăng", L"Hướng Hóa", L"Triệu Phong", L"Vĩnh Linh"}));
			data->Add(L"Thừa Thiên Huế", TaoList(gcnew cli::array<String^>{L"TP. Huế", L"TX. Hương Thủy", L"TX. Hương Trà", L"A Lưới", L"Nam Đông", L"Phong Điền", L"Phú Lộc", L"Phú Vang", L"Quảng Điền"}));

			// =================================================================================
			// 4. DUYÊN HẢI NAM TRUNG BỘ
			// =================================================================================
			data->Add(L"Đà Nẵng", TaoList(gcnew cli::array<String^>{L"Hải Châu", L"Thanh Khê", L"Sơn Trà", L"Ngũ Hành Sơn", L"Liên Chiểu", L"Cẩm Lệ", L"Hòa Vang", L"Hoàng Sa"}));
			data->Add(L"Quảng Nam", TaoList(gcnew cli::array<String^>{L"TP. Tam Kỳ", L"TP. Hội An", L"TX. Điện Bàn", L"Bắc Trà My", L"Đại Lộc", L"Đông Giang", L"Duy Xuyên", L"Hiệp Đức", L"Nam Giang", L"Nam Trà My", L"Nông Sơn", L"Núi Thành", L"Phú Ninh", L"Phước Sơn", L"Quế Sơn", L"Tây Giang", L"Thăng Bình", L"Tiên Phước"}));
			data->Add(L"Quảng Ngãi", TaoList(gcnew cli::array<String^>{L"TP. Quảng Ngãi", L"TX. Đức Phổ", L"Ba Tơ", L"Bình Sơn", L"Lý Sơn", L"Minh Long", L"Mộ Đức", L"Nghĩa Hành", L"Sơn Hà", L"Sơn Tây", L"Sơn Tịnh", L"Trà Bồng", L"Tư Nghĩa"}));
			data->Add(L"Bình Định", TaoList(gcnew cli::array<String^>{L"TP. Quy Nhơn", L"TX. An Nhơn", L"TX. Hoài Nhơn", L"An Lão", L"Hoài Ân", L"Phù Cát", L"Phù Mỹ", L"Tây Sơn", L"Tuy Phước", L"Vân Canh", L"Vĩnh Thạnh"}));
			data->Add(L"Phú Yên", TaoList(gcnew cli::array<String^>{L"TP. Tuy Hòa", L"TX. Đông Hòa", L"TX. Sông Cầu", L"Đồng Xuân", L"Phú Hòa", L"Sơn Hòa", L"Sông Hinh", L"Tây Hòa", L"Tuy An"}));
			data->Add(L"Khánh Hòa", TaoList(gcnew cli::array<String^>{L"TP. Nha Trang", L"TP. Cam Ranh", L"TX. Ninh Hòa", L"Cam Lâm", L"Diên Khánh", L"Khánh Sơn", L"Khánh Vĩnh", L"Trường Sa", L"Vạn Ninh"}));
			data->Add(L"Ninh Thuận", TaoList(gcnew cli::array<String^>{L"TP. Phan Rang - Tháp Chàm", L"Bác Ái", L"Ninh Hải", L"Ninh Phước", L"Ninh Sơn", L"Thuận Bắc", L"Thuận Nam"}));
			data->Add(L"Bình Thuận", TaoList(gcnew cli::array<String^>{L"TP. Phan Thiết", L"TX. La Gi", L"Bắc Bình", L"Đức Linh", L"Hàm Tân", L"Hàm Thuận Bắc", L"Hàm Thuận Nam", L"Phú Quý", L"Tánh Linh", L"Tuy Phong"}));

			// =================================================================================
			// 5. TÂY NGUYÊN
			// =================================================================================
			data->Add(L"Kon Tum", TaoList(gcnew cli::array<String^>{L"TP. Kon Tum", L"Đắk Glei", L"Đắk Hà", L"Đắk Tô", L"Ia H'Drai", L"Kon Plông", L"Kon Rẫy", L"Ngọc Hồi", L"Sa Thầy", L"Tu Mơ Rông"}));
			data->Add(L"Gia Lai", TaoList(gcnew cli::array<String^>{L"TP. Pleiku", L"TX. An Khê", L"TX. Ayun Pa", L"Chư Păh", L"Chư Prông", L"Chư Pưh", L"Chư Sê", L"Đak Đoa", L"Đak Pơ", L"Đức Cơ", L"Ia Grai", L"Ia Pa", L"Kbang", L"Kông Chro", L"Krông Pa", L"Mang Yang", L"Phú Thiện"}));
			data->Add(L"Đắk Lắk", TaoList(gcnew cli::array<String^>{L"TP. Buôn Ma Thuột", L"TX. Buôn Hồ", L"Buôn Đôn", L"Cư Kuin", L"Cư M'gar", L"Ea H'leo", L"Ea Kar", L"Ea Súp", L"Krông Ana", L"Krông Bông", L"Krông Búk", L"Krông Năng", L"Krông Pắc", L"Lắk", L"M'Drắk"}));
			data->Add(L"Đắk Nông", TaoList(gcnew cli::array<String^>{L"TP. Gia Nghĩa", L"Cư Jút", L"Đắk Glong", L"Đắk Mil", L"Đắk R'lấp", L"Đắk Song", L"Krông Nô", L"Tuy Đức"}));
			data->Add(L"Lâm Đồng", TaoList(gcnew cli::array<String^>{L"TP. Đà Lạt", L"TP. Bảo Lộc", L"Bảo Lâm", L"Cát Tiên", L"Di Linh", L"Đạ Huoai", L"Đạ Tẻh", L"Đam Rông", L"Đơn Dương", L"Đức Trọng", L"Lạc Dương", L"Lâm Hà"}));

			// =================================================================================
			// 6. ĐÔNG NAM BỘ
			// =================================================================================
			data->Add(L"Hồ Chí Minh", TaoList(gcnew cli::array<String^>{L"Thủ Đức", L"Quận 1", L"Quận 3", L"Quận 4", L"Quận 5", L"Quận 6", L"Quận 7", L"Quận 8", L"Quận 10", L"Quận 11", L"Quận 12", L"Bình Thạnh", L"Gò Vấp", L"Phú Nhuận", L"Tân Bình", L"Tân Phú", L"Bình Tân", L"Củ Chi", L"Hóc Môn", L"Bình Chánh", L"Nhà Bè", L"Cần Giờ"}));
			data->Add(L"Bình Phước", TaoList(gcnew cli::array<String^>{L"TP. Đồng Xoài", L"TX. Bình Long", L"TX. Phước Long", L"TX. Chơn Thành", L"Bù Đăng", L"Bù Đốp", L"Bù Gia Mập", L"Đồng Phú", L"Hớn Quản", L"Lộc Ninh", L"Phú Riềng"}));
			data->Add(L"Tây Ninh", TaoList(gcnew cli::array<String^>{L"TP. Tây Ninh", L"TX. Hòa Thành", L"TX. Trảng Bàng", L"Bến Cầu", L"Châu Thành", L"Dương Minh Châu", L"Gò Dầu", L"Tân Biên", L"Tân Châu"}));
			data->Add(L"Bình Dương", TaoList(gcnew cli::array<String^>{L"TP. Thủ Dầu Một", L"TP. Dĩ An", L"TP. Thuận An", L"TP. Tân Uyên", L"TP. Bến Cát", L"Bắc Tân Uyên", L"Bàu Bàng", L"Dầu Tiếng", L"Phú Giáo"}));
			data->Add(L"Đồng Nai", TaoList(gcnew cli::array<String^>{L"TP. Biên Hòa", L"TP. Long Khánh", L"Cẩm Mỹ", L"Định Quán", L"Long Thành", L"Nhơn Trạch", L"Tân Phú", L"Thống Nhất", L"Trảng Bom", L"Vĩnh Cửu", L"Xuân Lộc"}));
			data->Add(L"Bà Rịa - Vũng Tàu", TaoList(gcnew cli::array<String^>{L"TP. Vũng Tàu", L"TP. Bà Rịa", L"TX. Phú Mỹ", L"Châu Đức", L"Côn Đảo", L"Đất Đỏ", L"Long Điền", L"Xuyên Mộc"}));

			// =================================================================================
			// 7. ĐỒNG BẰNG SÔNG CỬU LONG
			// =================================================================================
			data->Add(L"Long An", TaoList(gcnew cli::array<String^>{L"TP. Tân An", L"TX. Kiến Tường", L"Bến Lức", L"Cần Đước", L"Cần Giuộc", L"Châu Thành", L"Đức Hòa", L"Đức Huệ", L"Mộc Hóa", L"Tân Hưng", L"Tân Thạnh", L"Tân Trụ", L"Thạnh Hóa", L"Thủ Thừa", L"Vĩnh Hưng"}));
			data->Add(L"Tiền Giang", TaoList(gcnew cli::array<String^>{L"TP. Mỹ Tho", L"TP. Gò Công", L"TX. Cai Lậy", L"Cái Bè", L"Cai Lậy", L"Châu Thành", L"Chợ Gạo", L"Gò Công Đông", L"Gò Công Tây", L"Tân Phước", L"Tân Phú Đông"}));
			data->Add(L"Bến Tre", TaoList(gcnew cli::array<String^>{L"TP. Bến Tre", L"Ba Tri", L"Bình Đại", L"Châu Thành", L"Chợ Lách", L"Giồng Trôm", L"Mỏ Cày Bắc", L"Mỏ Cày Nam", L"Thạnh Phú"}));
			data->Add(L"Trà Vinh", TaoList(gcnew cli::array<String^>{L"TP. Trà Vinh", L"TX. Duyên Hải", L"Càng Long", L"Cầu Kè", L"Cầu Ngang", L"Châu Thành", L"Duyên Hải", L"Tiểu Cần", L"Trà Cú"}));
			data->Add(L"Vĩnh Long", TaoList(gcnew cli::array<String^>{L"TP. Vĩnh Long", L"TX. Bình Minh", L"Bình Tân", L"Long Hồ", L"Mang Thít", L"Tam Bình", L"Trà Ôn", L"Vũng Liêm"}));
			data->Add(L"Đồng Tháp", TaoList(gcnew cli::array<String^>{L"TP. Cao Lãnh", L"TP. Sa Đéc", L"TP. Hồng Ngự", L"Cao Lãnh", L"Châu Thành", L"Hồng Ngự", L"Lai Vung", L"Lấp Vò", L"Tam Nông", L"Tân Hồng", L"Thanh Bình", L"Tháp Mười"}));
			data->Add(L"An Giang", TaoList(gcnew cli::array<String^>{L"TP. Long Xuyên", L"TP. Châu Đốc", L"TX. Tân Châu", L"TX. Tịnh Biên", L"An Phú", L"Châu Phú", L"Châu Thành", L"Chợ Mới", L"Phú Tân", L"Thoại Sơn", L"Tri Tôn"}));
			data->Add(L"Kiên Giang", TaoList(gcnew cli::array<String^>{L"TP. Rạch Giá", L"TP. Hà Tiên", L"TP. Phú Quốc", L"An Biên", L"An Minh", L"Châu Thành", L"Giang Thành", L"Giồng Riềng", L"Gò Quao", L"Hòn Đất", L"Kiên Hải", L"Kiên Lương", L"Tân Hiệp", L"U Minh Thượng", L"Vĩnh Thuận"}));
			data->Add(L"Cần Thơ", TaoList(gcnew cli::array<String^>{L"Ninh Kiều", L"Bình Thủy", L"Cái Răng", L"Ô Môn", L"Thốt Nốt", L"Cờ Đỏ", L"Phong Điền", L"Thới Lai", L"Vĩnh Thạnh"}));
			data->Add(L"Hậu Giang", TaoList(gcnew cli::array<String^>{L"TP. Vị Thanh", L"TP. Ngã Bảy", L"TX. Long Mỹ", L"Châu Thành", L"Châu Thành A", L"Long Mỹ", L"Phụng Hiệp", L"Vị Thủy"}));
			data->Add(L"Sóc Trăng", TaoList(gcnew cli::array<String^>{L"TP. Sóc Trăng", L"TX. Ngã Năm", L"TX. Vĩnh Châu", L"Châu Thành", L"Cù Lao Dung", L"Kế Sách", L"Long Phú", L"Mỹ Tú", L"Mỹ Xuyên", L"Thạnh Trị", L"Trần Đề"}));
			data->Add(L"Bạc Liêu", TaoList(gcnew cli::array<String^>{L"TP. Bạc Liêu", L"TX. Giá Rai", L"Đông Hải", L"Hòa Bình", L"Hồng Dân", L"Phước Long", L"Vĩnh Lợi"}));
			data->Add(L"Cà Mau", TaoList(gcnew cli::array<String^>{L"TP. Cà Mau", L"Cái Nước", L"Đầm Dơi", L"Năm Căn", L"Ngọc Hiển", L"Phú Tân", L"Thới Bình", L"Trần Văn Thời", L"U Minh"}));

			return data;
		}

	private:
		// Hàm phụ trợ giúp code ngắn gọn hơn
		static List<String^>^ TaoList(cli::array<String^>^ arr) {
			List<String^>^ list = gcnew List<String^>();
			list->AddRange(arr);
			return list;
		}
	};
}