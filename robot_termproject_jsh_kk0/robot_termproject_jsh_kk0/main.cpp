#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <time.h>
#include "Console.h"
#include "serialcomm.h"
#include <iostream>

using namespace std;

#define MAX 30
#define BACKCOLOR 15
#define DATA_LENGTH 4
#define BULLET_NUM 5
#define GAME_SLOW 100
#define GAME_FAST 5

struct ST_OBJECT {
	int x = 0;
	int y = 0;
	bool bActive = false;
};

void get_ADC(void);
void Send_data(BYTE data);

int g_iScore;
ST_OBJECT g_stEnemy[MAX];
ST_OBJECT g_stPlayer;
ST_OBJECT g_bullet[BULLET_NUM];

CSerialComm serialComm;
BYTE g_data;
BYTE* g_data_ptr;
BYTE g_recvData[DATA_LENGTH];	//�� ���������� ���⿡ ���ڸ����� 4���� ��������.

int g_Potentiometer = 5;
int g_lux_filtered;
int g_Coin;
int g_Tth_packet;
int g_UltraSonic_filtered_packet;
int g_PSD ;
int g_Press ;
int g_Gyro ;

void Spawn(void) {

	for (int i = 0; i < g_Potentiometer * 3; i++) {
		if (!g_stEnemy[i].bActive) {
			g_stEnemy[i].x = (rand() % 15) * 2;
			g_stEnemy[i].y = 0;
			g_stEnemy[i].bActive = true;
			break;
		}
	}
}

void InputProcess(void) {
	Send_data('p');

	get_ADC();

	g_lux_filtered = g_recvData[0];
	g_PSD = g_recvData[1];
	g_UltraSonic_filtered_packet = g_recvData[2];
	g_Gyro = g_recvData[3];
	
	if (GetAsyncKeyState(VK_UP) & 0x8000 || g_PSD == 0) {
	
		if (g_stPlayer.y < 1) g_stPlayer.y = 1;
		g_stPlayer.y--;
	}
	  
	if (GetAsyncKeyState(VK_DOWN) & 0x8000 || g_PSD == 9) {
		if (g_stPlayer.y > 28) g_stPlayer.y = 28;
		g_stPlayer.y++;
	}

	//non blocking
	if (GetAsyncKeyState(VK_LEFT) & 0x8000 || g_Gyro == 9) {//0x8000 : ȣ���� ������ ���� ȣ������� ��������
		g_stPlayer.x--;
		if (g_stPlayer.x < 0) g_stPlayer.x = 0;
	}

	if (GetAsyncKeyState(VK_RIGHT) & 0x8000 || g_Gyro == 0) {
		g_stPlayer.x++;
		if (g_stPlayer.x > 28) g_stPlayer.x = 28;
	}


	//���⿡ VK_SPACE ��ſ� �������� �Է¹޾Ƽ� �Ѿ� ������ ��.
	if (GetAsyncKeyState(VK_SPACE) & 0x8000 || g_lux_filtered == 1) {
		for (int i = 0; i < BULLET_NUM; i++) {
			if (g_bullet[i].bActive == false) {
				g_bullet[i].bActive = true;
				g_bullet[i].x = g_stPlayer.x;
				g_bullet[i].y = g_stPlayer.y;
				break;
			}
				
		}
	}
}

void UpdateBullet(void) {
	for (int i = 0; i < BULLET_NUM; i++) {
		if (g_bullet[i].bActive) {
			g_bullet[i].y--;
			GotoXY(g_bullet[i].x, g_bullet[i].y);
			printf("��");
			
		}
		if (g_bullet[i].y < 1)
			g_bullet[i].bActive = false;
	}
}
int EnemyProcess(void) {
	int count = 0;
	//�� ���� �Ź� ������Ʈ(�� ���������ϴ°�)
	for (int i = 0; i < MAX; i++) {
		if (g_stEnemy[i].bActive) {
			count++;
			GotoXY(g_stEnemy[i].x, g_stEnemy[i].y);
			printf("��");
			g_stEnemy[i].y++;

			//�Ѿ˰��� �浹ó��
			for (int j = 0; j < BULLET_NUM; j++) {
				if (g_stEnemy[i].y == g_bullet[j].y && g_stEnemy[i].x == g_bullet[j].x) {
					g_bullet[j].bActive = false;
					g_stEnemy[i].bActive = false;
				}  
				
			}
			
			//�浹ó��
			//cds �� ������ �������°� ��
			if (g_stEnemy[i].y == g_stPlayer.y && g_stEnemy[i].x == g_stPlayer.x && g_Press<4) {
				g_stPlayer.bActive = false;
			}

			if (g_stEnemy[i].y > 28) {
				g_stEnemy[i].bActive = false;
			}

			
		}
	}
	return count;
}

void Init(void) {
	int state = 1;

	g_stPlayer.x = 14, g_stPlayer.y = 28;
	g_stPlayer.bActive = true;
	srand(time(NULL));

	SetConsoleSize(30, 30);
}

void Update_player(void) {

	//���� ���� ���浹�Ѱ��
	if (g_stPlayer.bActive) {
		GotoXY(g_stPlayer.x, g_stPlayer.y);
		if(g_UltraSonic_filtered_packet)
			printf("��");
	}
	//���� �浹�Ѱ��
	else
	{
		GotoXY(g_stPlayer.x, g_stPlayer.y);
		printf("��");
	}
}

void Score() {
	if (g_stPlayer.bActive) g_iScore++;
	GotoXY(0, 0);
	printf("���ھ� : %d", g_iScore);
}

void StartMenu(void) {

	while (1) {
		Clear();
		GotoXY(10, 9);
		printf("�� ���ϱ�");

		GotoXY(5, 20);
		printf("�⺻ ������ ���ּ���!");
		GotoXY(5, 21);
		printf("..");
		GotoXY(10, 25);
		printf("made by ����ȣ");
		GotoXY(18, 26);
		printf("��⿵");


		if (_kbhit()) {
			
			Send_data('s');
			get_ADC();

			g_Potentiometer = g_recvData[0];
			g_Coin = g_recvData[1];
			g_Tth_packet = g_recvData[2];
			g_Press = g_recvData[3];
			
			break;		//���� ���� ����ȭ������ ���°ɷ� �ٲ����
		}
		Sleep(100);
	}
}

void GameMain(void) {
	
}

//
void Send_data(BYTE data) {
	
	if (!serialComm.sendCommand(data))
	{
		printf("send command failed\n");
	}
	
	
}

void Connect_Uart_Port(const char* _portNum) {
	if (!serialComm.connect(_portNum))
	{
		cout << "connect faliled";
		//return -1;
		return;
	}
}

void DisConnect_Uart_Port() {
	serialComm.disconnect();
}

//4�ڸ� ����.
//�� �Լ��� ������ g_recvData[]�� �ϳ��� �����.
void get_ADC(void){
		
	if (!serialComm.recvCommand(&g_data,4))
	{
		cout << "send command failed\n";
	}

	g_data_ptr = &g_data;

	for (int i = 0; i < DATA_LENGTH; i++) {
		g_recvData[i] = g_data_ptr[i] - 48;
	}

}


int main(void) {
	
	int data = 0;
	int pause = 1;

	Connect_Uart_Port("COM6");

	int state = 1;
	SetTitle("��������!");
	Init();
	SetColor(BACKCOLOR, 0);
	
	StartMenu();	//���Ⱑ �Ѿ������ ������ �ϸ� ��
	
	//GameMain();

	while (1) {
		
		state = 1;

		Init();

		while (1) {
			
			Clear();

			if (g_stPlayer.bActive) {
				//���� ����
				Spawn();
			}

			//input ó��
			InputProcess();
			UpdateBullet();

			state = EnemyProcess();
			//if (potentiometer > 10) {
			//	// �� ó��
			//	state = EnemyProcess();
			//}
			
			// player ���
			Update_player();

			Score();

			if (g_Tth_packet > 3)	Sleep(GAME_FAST);
				
			else	Sleep(GAME_SLOW);
			
			if (state == 0) break;
		}

		int flag = 0;
		
		if (g_Coin == 0) {
			printf("\n����� ������ ����˴ϴ�.");
			Sleep(1500);
			break;
		}
		else if (g_Coin == 1) {
			printf("\nCoin�� �ԷµǾ� ����� ������ �ٽ� ���۵˴ϴ�.");
			Sleep(1500);
			continue;
		}

	}

	DisConnect_Uart_Port();

	printf("\n");
	system("pause");
	return 0;

}

