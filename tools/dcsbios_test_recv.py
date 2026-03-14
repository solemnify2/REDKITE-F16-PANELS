"""
DCS-BIOS UDP 수신 테스트
UDP 데이터가 오는지만 확인합니다.
"""

import socket
import struct

MULTICAST_GROUP = "239.255.50.10"
MULTICAST_PORT = 5010

# 멀티캐스트 테스트
print(f"[1] 멀티캐스트 {MULTICAST_GROUP}:{MULTICAST_PORT} 테스트...")
sock1 = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
sock1.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
sock1.bind(("", MULTICAST_PORT))
mreq = struct.pack("4s4s", socket.inet_aton(MULTICAST_GROUP), socket.inet_aton("0.0.0.0"))
sock1.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, mreq)
sock1.settimeout(5.0)

try:
    data, addr = sock1.recvfrom(4096)
    print(f"    [OK] 멀티캐스트 수신 성공! from {addr}, {len(data)} bytes")
    print(f"    첫 20바이트: {data[:20].hex(' ')}")
    sock1.close()
except socket.timeout:
    print("    [X] 멀티캐스트 수신 실패 (5초 타임아웃)")
    sock1.close()

    # TCP 테스트
    print(f"\n[2] TCP localhost:7778 테스트...")
    try:
        sock2 = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock2.settimeout(3.0)
        sock2.connect(("127.0.0.1", 7778))
        print("    [OK] TCP 연결 성공! 데이터 대기 중...")
        sock2.settimeout(5.0)
        data = sock2.recv(4096)
        if data:
            print(f"    [OK] TCP 데이터 수신! {len(data)} bytes")
            print(f"    첫 20바이트: {data[:20].hex(' ')}")
        else:
            print("    [X] TCP 연결됐지만 데이터 없음")
        sock2.close()
    except (ConnectionRefusedError, OSError) as e:
        print(f"    [X] TCP 연결 실패: {e}")
    except socket.timeout:
        print("    [X] TCP 데이터 수신 실패 (5초 타임아웃)")

    # 일반 UDP 테스트
    print(f"\n[3] UDP localhost:7778 테스트...")
    sock3 = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock3.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    sock3.bind(("", 7778))
    sock3.settimeout(5.0)
    try:
        data, addr = sock3.recvfrom(4096)
        print(f"    [OK] UDP 7778 수신 성공! from {addr}, {len(data)} bytes")
        print(f"    첫 20바이트: {data[:20].hex(' ')}")
    except socket.timeout:
        print("    [X] UDP 7778 수신 실패 (5초 타임아웃)")
    sock3.close()

print("\n테스트 완료. DCS World에서 F-16 미션이 실행 중인 상태에서 다시 시도하세요.")
