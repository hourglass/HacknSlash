# HacknSlash
Unreal 프로젝트
## Weapon
[Weapon](https://github.com/hourglass/HacknSlash/blob/main/Source/HacknSlash/Weapon.cpp)  
- 무기의 정보를 관리하고 넘겨주는 클래스입니다.  
- 데이터테이블 에셋에서 정보를 받아와 클래스의 멤버 변수에 저장합니다.  
---
[WeaponManagerComponent](https://github.com/hourglass/HacknSlash/blob/main/Source/HacknSlash/WeaponManagerComponent.cpp)  
- 캐릭터가 장착한 무기를 관리하는 컴포넌트입니다.  
- 떨어진 무기를 줍고 장착하는 기능과 장착한 무기를 해제하는 기능이 있습니다.  
- 공격 성공 시 카메라 쉐이킹, 애니메이션 속도 조절, 맞은 대상의 넉백 기능을 수행합니다.  
- 애니메이션 동안 수행할 공격 충돌 체크 함수를 가지고 있습니다.  
- 공격 유형에 따라 다른 공격 충돌 체크를 수행합니다.  
---
[AttackCheckNotifyState](https://github.com/hourglass/HacknSlash/blob/main/Source/HacknSlash/AttackCheckNotifyState.cpp)  
- 몽타주의 애니메이션 틱과 틱 종료시 이벤트를 수행하는 클래스입니다.  
- 공격 관련 인터페이스의 함수를 호출하여 공격 충돌 체크와 체크 종료 시 이벤트를 실행합니다.  
---
[HitReactionComponent](https://github.com/hourglass/HacknSlash/blob/main/Source/HacknSlash/HitReactionComponent.cpp)  
- 공격 시 타격감을 주기 위한 기능이 구현된 컴포넌트 입니다.  
- 커브 데이터의 값을 따라 메쉬의 로케이션을 변경하여 피격 시 진동을 수행합니다.  
- 커브 데이터의 값을 따라 CustomTimeDilation 값을 변경하여 피격 시 경직을 수행합니다.  
- 커브 데이터의 값을 따라 메쉬의 머티리얼 값을 변경하여 피격 시 림라이팅 효과를 수행합니다.  
---
## Player
[HnsCharacter](https://github.com/hourglass/HacknSlash/blob/main/Source/HacknSlash/HnsCharacter.cpp)  
- 플레이어의 조작을 담당하는 클래스입니다.  
- 이동, 공격, 구르기, 무기 장착 & 해제 기능을 수행합니다.
- 마우스 좌클릭을 연속으로 누르면 콤보 공격을 수행합니다.  
- 플레이어의 상태는 enum을 사용해 관리했습니다. (Idle, Attack, Roll, Hit)  
- 피격 시 처리 함수를 수행하고 UI를 갱신합니다.  
---
## Enemy
[Enemy](https://github.com/hourglass/HacknSlash/blob/main/Source/HacknSlash/Enemy.cpp)  
- 적군을 조종하는 클래스입니다.  
- 적군의 상태는 enum을 사용해 관리했습니다. (Spawned, Idle, Chase, Attack, Hit)  
- 현재 상태에 따라 BehaviorTree를 갱신합니다.  
- BehaviorTree의 Task를 통해 플레이어 추적, 공격, 뒷걸음질, 무기 줍기를 수행합니다.  
---
[AIBehaviorInterface.h](https://github.com/hourglass/HacknSlash/blob/main/Source/HacknSlash/AIBehaviorInterface.h)  
[BTT_TryAction](https://github.com/hourglass/HacknSlash/blob/main/Source/HacknSlash/BTT_TryAction.cpp)  
[BTT_FocusTarget](https://github.com/hourglass/HacknSlash/blob/main/Source/HacknSlash/BTT_FocusTarget.cpp)  
[BTT_AttackTarget](https://github.com/hourglass/HacknSlash/blob/main/Source/HacknSlash/BTT_AttackTarget.cpp)  
[BTT_SetRecoilLocation](https://github.com/hourglass/HacknSlash/blob/main/Source/HacknSlash/BTT_SetRecoilLocation.cpp)  
[BTT_FindWeapon](https://github.com/hourglass/HacknSlash/blob/main/Source/HacknSlash/BTT_FindWeapon.cpp)  
[BTT_EquipWeapon](https://github.com/hourglass/HacknSlash/blob/main/Source/HacknSlash/BTT_EquipWeapon.cpp)  
