#include "stdafx.h"

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <math.h>
#include <Windows.h>
#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable:4996)

/*
1.먼저 딜러를 정한 뒤, 베팅을 한다. - 딜러는 자동으로 정해짐, 베팅 구현
2.딜러가 자신을 포함한 참가자 전원에게 카드 두 장을 나누어주는데, 딜러의 카드 한 장은 상대에게 보이지 않는다.
3.카드의 합이 딜러보다 먼저 21이 되거나 딜러보다 21에 가깝게 되면 이기고, 카드를 더 받았는데 21을 초과하면 버스트(Bust)된다.
4.먼저 받은 카드 두 장의 합이 21에 못 미치면 히트(Hit)라고 말한 뒤 한 장씩 더 받을 수 있고, 멈추려면 스탠드(Stand)라고 말한다.
5.딜러는 카드의 합이 16 이하면 무조건 한 장을 더 받아야 하고, 17 이상의 경우에는 멈추어야 한다.
6.딜러의 카드와 합이 같으면 비긴 것이 된다.
7.에이스 카드는 1이나 11로 취급할 수 있고, 10, J, Q, K는 모두 10으로 계산한다.
8.처음 받은 카드 두 장이 에이스와 10, J, Q, K 중의 하나로 합이 21이 되면 블랙잭(Blackjack)이 되고, 베팅한 금액의 1.5배의 돈을 받는다.
*/

struct Card {
	int Num = 0; // 0(비어있음) 1(A) 2 3 4 5 6 7 8 9 10 11(J) 12(Q) 13(K)
	int Shape; // 0 = 스페이드, 1 = 클로버, 2 = 다이아몬드, 3 = 하트
};
char ch_shape_sp[3] = "♤";
char ch_shape_cl[3] = "♧";
char ch_shape_di[3] = "◆";
char ch_shape_he[3] = "♥";
int Money = 10000; // 시작금액 10000원
int Cardpool[4][13]; // 그 카드를 뽑았는지 아닌지를 처리 1 : 존재, 0 : 없음

Card Dealer[10]; // 딜러가 가지고있는 카드
Card Player[10]; // 플레이어가 가지고있는 카드

void load_money();
void save_money();

// cmd에서 깜빡이는 커서숨김. Concealing cursor from cmd.
void removeCursorFromCMD() {
	CONSOLE_CURSOR_INFO CurInfo;
	CurInfo.dwSize = 1;
	CurInfo.bVisible = FALSE;
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &CurInfo);
}

// 입력버퍼 지우기
void Clearbuff()
{
	while (getchar() != '\n');
}

// Cardpool에 모든 카드가 존재하는걸로 리셋, 딜러와 플레이어의 카드도 리셋
void ResetCardpool()
{
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 13; j++)
			Cardpool[i][j] = 1;
	for (int i = 0; i < 10; i++)
	{
		Dealer[i].Num = 0;
		Player[i].Num = 0;
	}
}

// Card를 랜덤으로 생성 (cardpool에 없는 카드는 나오지 않는다)
Card RandomCard()
{
	do
	{
		int Num = rand() % 13 + 1;
		int Shape = rand() % 4;
		if (Cardpool[Shape][Num-1] == 1)
		{
			Cardpool[Shape][Num-1] = 0;
			return { Num, Shape };
		}
	} while (1);
}

// 카드를 표시하는 함수
void Cardprint(Card hand[], int num, int hidden)
{
	char ch_shape[10][3];
	char ch_num[10][3];
	for (int i = 0; i < num; i++)
	{
		switch (hand[i].Shape)
		{
		case 0: //spade
			strcpy(ch_shape[i], ch_shape_sp);
			break;
		case 1: //clover
			strcpy(ch_shape[i], ch_shape_cl);
			break;
		case 2: //diamond
			strcpy(ch_shape[i], ch_shape_di);
			break;
		default: //heart
			strcpy(ch_shape[i], ch_shape_he);
			break;
		}

		if (1 < hand[i].Num && hand[i].Num < 10)
			sprintf(ch_num[i], " %d", hand[i].Num);
		else if (hand[i].Num == 1) strcpy(ch_num[i], " A");
		else if (hand[i].Num == 10) strcpy(ch_num[i], "10");
		else if (hand[i].Num == 11) strcpy(ch_num[i], " J");
		else if (hand[i].Num == 12) strcpy(ch_num[i], " Q");
		else if (hand[i].Num == 13) strcpy(ch_num[i], " K");
	}

		for (int j = 0; j < num; j++)
			printf(" ┌──────┐ ");
		printf("\n");
		for (int j = 0; j < num - hidden; j++)
			printf(" │ %s %s│ ", ch_shape[j], ch_num[j]);
		for (int j = 0; j < hidden; j++)
			printf(" │ XXXXX│ ");
		printf("\n");
		for (int j = 0; j < num; j++)
			printf(" └──────┘ ");
		printf("\n");
}

// 타겟에게 카드를 주는 함수
void DrawCard(Card target[])
{
	int i;
	for (i = 0; target[i].Num != 0; i++);
	if (i == 10) return;
	target[i] = RandomCard();
}

// 시작시 화면
int Mainscreen()
{
	int sel;

	system("cls");
	printf("\n  ├  BLACKJACK ┤\n\n");
	printf("  현재 %d원이 있습니다.\n\n", Money);
	printf("  [ 0] How to play Blackjack\n");
	printf("  [!0] Play Blackjack\n");
	printf("  [-1] Quit\n\n  >> ");
	while (1)
	{
		if (scanf("%d", &sel) != 0)
		{
			Clearbuff();
			break;
		}
		printf("  └ 다시 입력해주세요 >> ");
		Clearbuff();
	}
	if (sel == 0)
	{
		system("cls");
		printf("\n   1. 먼저 딜러를 정한 뒤, 베팅을 한다. - 딜러는 컴퓨터가 합니다.\n\n\
   2. 딜러가 자신을 포함한 참가자 전원에게 카드 두 장을 나누어주는데, 딜러의 카드 한 장은 상대에게 보이지 않는다.\n\n\
   3. 카드의 합이 딜러보다 먼저 21이 되거나 딜러보다 21에 가깝게 되면 이기고, 카드를 더 받았는데 21을 초과하면 버스트(Bust)된다.\n\n\
   4. 먼저 받은 카드 두 장의 합이 21에 못 미치면 히트(Hit)라고 말한 뒤 한 장씩 더 받을 수 있고, 멈추려면 스탠드(Stand)라고 말한다.\n\n\
   5. 딜러는 카드의 합이 16 이하면 무조건 한 장을 더 받아야 하고, 17 이상의 경우에는 멈추어야 한다.\n\n\
   6. 딜러의 카드와 합이 같으면 비긴 것이 된다.\n\n\
   7. 에이스 카드는 1이나 11로 취급할 수 있고, 10, J, Q, K는 모두 10으로 계산한다.\n\n\
   8. 처음 받은 카드 두 장이 에이스와 10, J, Q, K 중의 하나로 합이 21이 되면 블랙잭(Blackjack)이 되고, 베팅한 금액의 1.5배의 돈을 받는다.\n\n\
\n 출처 : 위키피디아 블랙잭 문서\n\n");
		_getch();
		return Mainscreen();
	}
	else if (sel == -1)
	{
		system("cls");
		save_money();
		printf("\n  ");
		exit(0);
	}
	else
	{
		int bet;
		system("cls");
		printf("\n  ├  BLACKJACK ┤\n\n");
		printf("  현재 %d원이 있습니다.\n\n", Money);
		printf("  베팅 금액 : ");
		while (1)
		{
			if (scanf("%d", &bet) != 0 && bet <= Money)
			{
				Clearbuff();
				Money -= bet;
				save_money();
				return bet;
			}
			printf("  └ 다시 입력해주세요 : ");
			Clearbuff();
		}
	}
}

// 블랙잭인지 여부를 확인
int isBlackJack()
{
	int fir = Player[0].Num;
	int sec = Player[1].Num;
	int sum = fir + sec;
	if (sum >= 11 && ((fir == 1) || (sec == 1))) return 1;
	return 0;
}

// 보유한 카드들의 합을 리턴하는 함수, A를 1로할지 11로할지 결정, 10 이상은 모두 10으로 처리
int CardSum(Card Target[])
{
	int cont; // Target의 카드의 수
	for (cont = 0; Target[cont].Num != 0; cont++);
	int sum = 0; // Target의 카드들의 합
	for (int i = 0; i < cont; i++)
	{
		switch (Target[i].Num)
		{
		case 1:
			int k;
			do
			{
				printf("\n  %d번째 A를 1과 11중 어느것으로 하시겠습니까? (1/11) : ", i + 1);
				scanf("%d", &k);
				Clearbuff();
			} while (!(k == 1 || k == 11));
			sum += k;
			break;
		case 11:
		case 12:
		case 13:
			sum += 10;
			break;
		default:
			sum += Target[i].Num;
			break;
		}
	}
	return sum;
}

// 딜러만을 위한 CardSum. A를 자율적으로 판단한다.
int DealerCardSum()
{
	int cont; // Target의 카드의 수
	for (cont = 0; Dealer[cont].Num != 0; cont++);
	int sum[5] = { 0,10,20,30,40 }; // Target의 카드들의 합, A를 1로 판단한 개수에 따라 변동
	int num_A = 0;
	for (int i = 0; i < cont; i++)
	{
		switch (Dealer[i].Num)
		{
		case 1:
			for (int j = 0; j < 5; j++) sum[j] += 1;
			num_A++;
			break;
		case 11:
		case 12:
		case 13:
			for (int j = 0; j < 5; j++) sum[j] += 10;
			break;
		default:
			for (int j = 0; j < 5; j++) sum[j] += Dealer[i].Num;
			break;
		}
	}
	// 5개의 sum경우중 21에 가장 가까운걸 찾는다.
	int sum_min = 50;
	int sum_min_index = 0;
	int diff[5];
	for (int i = 0; i <= num_A; i++)
	{
		diff[i] = sum[i] - 21 <= 0 ? abs(sum[i] - 21) : 50;
		if (sum_min > diff[i])
		{
			sum_min = diff[i];
			sum_min_index = i;
		}
	}
	return sum[sum_min_index];
}

// 딜러가 카드를 뽑는 AI, 16 이하면 반드시 드로우. 17이상은 정지
int DealerAI(int playersum, int bet, int PlayerN, int DealerN)
{
	int Dealersum = DealerCardSum();
	// 1.7초 대기
	Sleep(1700);
	// 화면에 표시
	system("cls");
	printf("\n  %d원을 베팅하셨습니다. (남은 돈 : %d원)\n\n", bet, Money);
	printf("\n ┌─  유저의 카드 ─┐ \n");
	Cardprint(Player, PlayerN, 0);
	printf("\n  숫자 합 : %d\n\n", playersum);
	printf(" ┌─  딜러의 카드 ─┐ \n");
	Cardprint(Dealer, DealerN, 0);
	printf("\n  현재 딜러 카드의 합 : %d\n", Dealersum);
	// 16 이하면 반드시 드로우. 17이상은 정지, 딜러가 버스트할 경우
	if (Dealersum <= 16)
	{
		// 추가 카드 뽑기
		Dealer[DealerN++] = RandomCard();
		// 화면에 표시
		printf("\n  딜러가 카드를 뽑습니다.\n");
		Dealersum = DealerAI(playersum, bet, PlayerN, DealerN);
	}
	if (Dealersum > 21)
	{
		printf("\n  딜러의 Bust!\n");
		return 0;
	}
	return Dealersum;
}

// 메인 게임부분, 출력값은 게임이 끝난 후 받은 금액
int Maingame(int bet)
{
	int DealerN = 2; // 딜러의 카드수
	int PlayerN = 2; // 플레이어의 카드수

	ResetCardpool();
	system("cls");
	printf("\n  %d원을 베팅하셨습니다. (남은 돈 : %d원)\n\n", bet, Money);
	DrawCard(Dealer); DrawCard(Dealer);
	printf(" ┌─  딜러의 카드 ─┐ \n");
	Cardprint(Dealer, DealerN, 1);
	_getch();
	DrawCard(Player); DrawCard(Player);
	printf("\n ┌─  유저의 카드 ─┐ \n");
	Cardprint(Player, PlayerN, 0);
	if (isBlackJack()) // 블랙잭이면 배팅액의 1.5배
	{
		printf("\n  BLACKJACK!\n\n");
		return bet * 5 / 2;
	}
	int playersum;
	do
	{
		printf("\n  카드를 더 받으시겠습니까? (Hit or Stand) (Y/N) : ");
		char input = getchar();
		Clearbuff();

		if (input == 'Y' || input == 'y')
		{
			DrawCard(Player);
			system("cls");
			printf("\n  %d원을 베팅하셨습니다. (남은 돈 : %d원)\n\n", bet, Money);
			printf(" ┌─  딜러의 카드 ─┐ \n");
			Cardprint(Dealer, DealerN, 1);
			printf("\n ┌─  유저의 카드 ─┐ \n");
			Cardprint(Player, ++PlayerN, 0);
			playersum = CardSum(Player);
			if (playersum > 21)
			{
				printf("\n\n  Bust! (숫자가 21을 초과했습니다.)\n\n  Dealer의 승리\n\n  ");
				return 0;
			}
			else if (playersum == 21)
			{
				printf("\n\n  Player의 승리. (먼저 21에 도달했습니다.)\n\n  ");
				return bet * 2;
			}
		}
		else if (input == 'N' || input == 'n') break;
	} while (1);

	// 유저 카드 오픈, 합산
	system("cls");
	printf("\n  %d원을 베팅하셨습니다. (남은 돈 : %d원)\n\n", bet, Money);
	printf("\n ┌─  유저의 카드 ─┐ \n");
	Cardprint(Player, PlayerN, 0);
	playersum = CardSum(Player);
	printf("\n  숫자 합 : %d\n\n", playersum);
	// 딜러 카드 오픈, 딜러의 드로우
	printf(" ┌─  딜러의 카드 ─┐ \n");
	Cardprint(Dealer, DealerN, 0);
	int Dealersum = DealerCardSum();
	printf("\n  현재 딜러 카드의 합 : %d", Dealersum);
	Dealersum = DealerAI(playersum, bet, PlayerN, DealerN);
	// 승리 판정
	int diff[2] = { playersum - 21, Dealersum - 21 }; // 이중 더 큰값이 승리
	if (diff[0] > diff[1] && diff[0] <= 0)
	{
		printf("\n\n  Player의 승리. (플레이어의 합이 21에 더 가깝습니다.)\n\n  ");
		return bet * 2;
	}
	else if (diff[0] == diff[1])
	{
		printf("\n\n  무승부. (플레이어의 합과 딜러의 합이 같습니다.)\n\n  ");
		return bet;
	}
	else
	{
		printf("\n\n  Dealer의 승리. (딜러의 합이 21에 더 가깝습니다.)\n\n  ");
		return 0;
	}
}

// 돈을 불러오는 함수.
void load_money()
{
	FILE *fp;
	int temp_money;
	if ((fp = fopen("Money.bin", "r+")) != NULL) // 돈 파일이 존재할 때
	{
		fscanf(fp, "%d", &temp_money);
		if (temp_money == 0)
		{
			printf("\n  길을 가던 도중 1000원을 주웠습니다...\n\n\n  아무 키나 눌러 진행합니다.");
			_getch();
			temp_money = 1000;
		}
		Money = temp_money;
	}
	else // 돈 파일이 존재하지 않을 때
	{
		fp = fopen("Money.bin", "w");
		fprintf(fp, "10000");
		Money = 10000;
	}
	fclose(fp);
}

// 돈을 저장하는 함수
void save_money()
{
	FILE *fp = fopen("Money.bin", "w");
	fprintf(fp, "%d", Money);
	fclose(fp);
}

int main()
{
	removeCursorFromCMD();
	srand(time(NULL));
	load_money();
	while (1)
	{
		Money += Maingame(Mainscreen());
		save_money();
		system("PAUSE");

		if (Money == 0)
		{
			break;
		}
	}
	system("cls");
	printf("\n  돈이 없어 쫒겨났습니다...\n\n\n  ");
    return 0;
}