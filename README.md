# e-paper.ch32v203
저가형 RISC-V(CH32V203)을 사용한 Vu12(HDMI2LVDS:1920x720) Project
* mono module : user screen 4개 저장
* color module : user screen 2개 저장
* ttyACM port를 활용하여 user image 전송가능 (한/영, bitmap데이터 등)

ch32v203 참조사이트
* WCH Product site : https://www.wch-ic.com/products/CH32V203.html
* WCH ISP Tool(wchisp) : https://github.com/ch32-rs/wchisp
* RISC-V Toolchain : http://www.mounriver.com/download (Linux 2024/04/03: MRS_Toolchain_Linux_x64_V1.91.tar.xz)
* Demo Board : https://github.com/WeActStudio/WeActStudio.BluePill-Plus-CH32
* Project Doc(Protocol & Port define) : https://docs.google.com/spreadsheets/d/1Dm8ncan42hRj0ewswMPVniGle1uVQBbU9usRldSD-SI/edit#gid=1152594250

ch32v203 write error
* isp download시 verify error 발생 이유는 flash protect가 되어 있어서 발생됨.
* wchisp tool을 사용하여 flash unprotect후 사용하여야 함. (wchisp config unprotect)

ch32v203 모델별 메모리 설정
* Ld 폴더안의 Link.ld내용을 수정

ch32V203 Debug serial port 설정
* Debug 폴더안의 debug.h 파일 내용중 DEBUG_UARTx 설정

e-paper module (2.9" 296x128, mono/color(red))
* Gitub: https://github.com/WeActStudio/WeActStudio.EpaperModule
* product: https://ko.aliexpress.com/item/1005004644515880.html?spm=a2g0o.detail.pcDetailBottomMoreOtherSeller.2.9d92h53Rh53Ruc&gps-id=pcDetailBottomMoreOtherSeller&scm=1007.40000.326746.0&scm_id=1007.40000.326746.0&scm-url=1007.40000.326746.0&pvid=78c0b815-53ae-4dd6-a3d7-8469fcd48f03&_t=gps-id:pcDetailBottomMoreOtherSeller,scm-url:1007.40000.326746.0,pvid:78c0b815-53ae-4dd6-a3d7-8469fcd48f03,tpp_buckets:668%232846%238115%23841&pdp_npi=4%40dis%21KRW%2111103%2111103%21%21%2157.51%2157.51%21%40214112dd17161811532316553ebafc%2112000031468032209%21rec%21KR%211614259518%21&utparam-url=scene%3ApcDetailBottomMoreOtherSeller%7Cquery_from%3A
 
