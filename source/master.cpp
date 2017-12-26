#include <iostream>
#include <sstream>
#include <istream>
#include <vector>
#include <algorithm> //min, max
#include <string.h> //memcpy

using namespace std;

// 入力に揃え、どの関数もカードはすべて1オリジン計算.

#define CARD_NUM_MAX	(13)    // カードの数値種類.
#define CARD_MARK_MAX	(4)     // カードのマーク種類.
#define BLACK_JACK      (21)	// 最強のスコア.
#define SCORE_BURST		(0)		// バーストしたスコア.
#define SCORE_INVALID	(-1)	// スコア無効.
#define BET_MARGIN_RATE		(3.0f)	// 期待値の何倍余裕をもってベットするか.
#define BET_CALC_WIN_RATE	(0.5f)	// ベットの計算に使う自分の勝率.
#define PERMISSIBLE_ERROR	( 0.00001f )	// 許容誤差.

#define MILLION		( 1000000LL )
#define BILLION		( 1000000000LL )

/*************************************************/
/* 基本のブラックジャックするクラス              */
/*************************************************/
class CPaizaJacker {
public:
	CPaizaJacker();
	virtual ~CPaizaJacker();

	virtual void AnalyzeInput(istream & istInput);

	virtual bool IsBet() const { return m_bIsBet; }
	virtual long long GetBet() const;

	virtual bool IsHitBetter() const;
protected:
	virtual	int GetParentStandScore() const { return 17; }
	virtual	long long GetBaseBetMax() const { return 40 * MILLION; }// 自動的にクリップされるのであまり気にしないでOK.
	virtual	long long GetBetMax() const { return 10 * BILLION; }// 自動的にクリップされるのであまり気にしないでOK.
	virtual	float GetBaseRewardRate() const { return 3.0f; }
	virtual bool IsParentHit(const vector<int>& vecMyCards, const vector<int>& vecYourCards, const vector<int>& vecRemainCards) const;

protected:
	int GetScore(const vector<int>& vecCards) const;
	float GetWinRate(int nScore, const float * pfParentScoreRate) const;
	void GetParentScoreRate(float * out_pfParentScoreRate, const vector<int>& vecMyCards, const vector<int>& vecYourCards, const vector<int>& vecRemainCards) const;
	float GetHitWinRate(const vector<int>& vecCards, const float * pfParentScoreRate, const vector<int>& vecRemainCards) const;

protected:
	// ベット用パラメータ.
	bool				m_bIsBet;		// Betかどうか.
	long long			m_llMyChip;		// 持っているチップ数.

	// カード用パラメータ.
	vector<int>			m_vecMyCard;	// 自分のカード.
	vector<int>			m_vecYourCard;	// 相手のカード.
	vector<int>			m_vecRemainCard;// 山のカード.

	// ベット、カード共通パラメータ.
	int				m_nGameCount;		// ゲームカウント.
	int				m_nWinCount;		// 連勝カウント.
	long long		m_llMaxBetCount;	// 最大ベット.

	static const float DEFAULT_PARENT_SCORE_RATE[BLACK_JACK+1];
};

// 親のスコア率(別プログラムで試行,四捨五入のおかげで1.0f超えるけど問題なし).
const float CPaizaJacker::DEFAULT_PARENT_SCORE_RATE[BLACK_JACK+1] = 
{
	0.282f,
	.0f, .0f, .0f, .0f, .0f, .0f, .0f, .0f, .0f, .0f, .0f, .0f, .0f, .0f, .0f, .0f,/* 1～16 */
	0.145f, //17
	0.140f, //18
	0.134f, //19
	0.179f, //20
	0.121f, //21
};

CPaizaJacker::CPaizaJacker() : 
	m_bIsBet(false),
	m_llMyChip(0LL),
	m_vecMyCard(),
	m_vecYourCard(),
	m_vecRemainCard(),
	m_nGameCount(0),
	m_nWinCount(0),
	m_llMaxBetCount(0LL)
{
}

CPaizaJacker::~CPaizaJacker()
{
}

void CPaizaJacker::AnalyzeInput( istream& istInput )
{
	string strInput;
	stringstream ss;
	string buffer;

	// 自分のカードor自分のチップ.
	vector<long long> vecFirstInput;
	ss.clear();
	getline(istInput, strInput);
	ss << strInput;
	while (getline(ss, buffer, ' ')) {
		vecFirstInput.push_back(stoll(buffer));
	}
	if(vecFirstInput[0] == 0LL) {
		// ベットだった.
		m_bIsBet = true;
		m_llMyChip = vecFirstInput[1];
	}
	else {
		for(auto itr = vecFirstInput.begin(); itr != vecFirstInput.end(); ++itr) {
			m_vecMyCard.push_back( (int)*itr );
		}
	}

	// ゲーム数.
	if( !istInput.eof() ) {
		vector<int> vecGameCount;
		ss.clear();
		getline(istInput, strInput);
		ss << strInput;
		while (getline(ss, buffer, ' ')) {
			vecGameCount.push_back(stoi(buffer));
		}
		m_nGameCount = vecGameCount[0];
	}

	// 連勝数.
	if( !istInput.eof() ) {
		vector<int> vecWinCount;
		ss.clear();
		getline(istInput, strInput);
		ss << strInput;
		while (getline(ss, buffer, ' ')) {
			vecWinCount.push_back(stoi(buffer));
		}
		m_nWinCount = vecWinCount[0];
	}

	// 最大ベット数.
	if( !istInput.eof() ) { 
		vector<long long> vecMaxBetCount;
		ss.clear();
		getline(istInput, strInput);
		ss << strInput;
		while (getline(ss, buffer, ' ')) {
			vecMaxBetCount.push_back(stoll(buffer));
		}
		m_llMaxBetCount = vecMaxBetCount[0];
	}

	// ベットの場合はここまで.
	if( m_bIsBet ) {
		return;
	}

	// 相手のカード.
	if( !istInput.eof() ) {
		ss.clear();
		getline(istInput, strInput);
		ss << strInput;
		while (getline(ss, buffer, ' ')) {
			m_vecYourCard.push_back(stoi(buffer));
		}
	}

	// 残りのカード.
	if( !istInput.eof() ) {
		ss.clear();
		getline(istInput, strInput);
		ss << strInput;
		while (getline(ss, buffer, ' ')) {
			m_vecRemainCard.push_back(stoi(buffer));
		}
	}

	if( m_vecRemainCard.size() == 0 ) {
		// 入力がない場合は1セット使っていると仮定.
		for(int iCard = 1 ; iCard <= CARD_NUM_MAX ; iCard++ ) {
			// 既に配られているカードは除く.
			int nOwnCard = count( m_vecMyCard.begin(), m_vecMyCard.end(), iCard ) + count( m_vecYourCard.begin(), m_vecYourCard.end(), iCard );
			// トランプ1セットじゃないかもしれないので0未満にはしない.
			int nCardCount = max( CARD_MARK_MAX - nOwnCard, 0 );
			for( int iPush = 0 ; iPush < nCardCount ; iPush++ ) {
				m_vecRemainCard.push_back( iCard );
			}
		}
	}
}

long long CPaizaJacker::GetBet() const
{
	long long llBet = 1LL;
	if( m_llMaxBetCount != 0LL ) {
		// 最大別途数がある場合は勝った額ベットできるパターン.
		llBet = GetBaseBetMax();
		float fMargin = (float)GetBaseBetMax() * BET_MARGIN_RATE / BET_CALC_WIN_RATE;
		// 特定連勝数に対して、余裕があるチップがあるなら賭けて良い.
		long long llMyChipBefore = m_llMyChip - m_llMaxBetCount;// 今得たものは計算に入れない(増えまくるので)
		if( (long long)fMargin < llMyChipBefore ) {
			fMargin /= BET_CALC_WIN_RATE;
			while( (long long)fMargin < llMyChipBefore ) {
				fMargin /= BET_CALC_WIN_RATE;
				llBet = (long long)( (float)llBet * GetBaseRewardRate() );
			}
			return min( min( m_llMaxBetCount, llBet ), GetBetMax() );
		}
	}

	// 最大ベット数がない場合は、最大ベットが固定値のパターン.
	// ベースの最大ベットまでチップがない場合もこっちで計算.
	llBet = min( max( (long long)( (float)m_llMyChip / BET_MARGIN_RATE * BET_CALC_WIN_RATE), 1LL ), GetBaseBetMax() );
	return min( m_llMyChip, llBet );
}

bool CPaizaJacker::IsHitBetter() const
{
	int nMyScore = GetScore( m_vecMyCard );
	int nYourScore = GetScore( m_vecYourCard );
	if( nYourScore == BLACK_JACK || nMyScore == BLACK_JACK ) {
		// 自分がブラックジャックor相手がブラックジャックなら引かない.
		return false;
	}
	else if( nYourScore == SCORE_BURST ) {
		// 相手がバーストしているなら引かない.
		return false;
	}
	else if( !IsParentHit( m_vecMyCard, m_vecYourCard, m_vecRemainCard ) ) {
		if( nMyScore <= nYourScore ) {
			// 相手がSTAND基準を超えていて、自分が負けている場合はHITしないと負ける.
			return true;
		}
		else {
			// 相手がSTAND基準を超えていて、自分が買っている場合はSTANDすれば勝てる.
			return false;
		}
	}
	else {
		// 今のスコアと勝率.
		float fParentScoreRate[BLACK_JACK+1] = { 0 };
		GetParentScoreRate( fParentScoreRate, m_vecMyCard, m_vecYourCard, m_vecRemainCard );
		float fNowWinRate = GetWinRate( nMyScore, fParentScoreRate );
		float fHitWinRate = GetHitWinRate( m_vecMyCard, fParentScoreRate, m_vecRemainCard );
		if( fHitWinRate >= fNowWinRate - PERMISSIBLE_ERROR/*許容誤差*/ ){ // 勝つ確率アップなら.
			return true;
		} else {
			return false;
		}
	}
}
// 親がヒットするか.
bool CPaizaJacker::IsParentHit(const vector<int>& vecMyCards, const vector<int>& vecYourCards, const vector<int>& vecRemainCards) const
{
	// 相手のカードは見ず、自分が17未満か.
	int nYourScore = GetScore( vecYourCards );
	return nYourScore == SCORE_INVALID || (nYourScore < GetParentStandScore() && nYourScore != SCORE_BURST && nYourScore != BLACK_JACK);
}
// そのカードのスコアを獲得.
// 戻り値 - スコア、0ならバースト.
int CPaizaJacker::GetScore( const vector<int> &vecCards ) const
{
	if( vecCards.size() == 0 ) {
		return SCORE_INVALID;
	}
    int nAce = 0;// ACE数.
    int nScoreSumExpectAce = 0;// ACE以外の合計スコア.
    for(auto itr = vecCards.begin(); itr != vecCards.end(); ++itr) {
	    if( *itr == 1 ) {
	        nAce++;
	    }
	    else if( *itr >= 11 ) {
	        nScoreSumExpectAce += 10;
        }
	    else {
            nScoreSumExpectAce += *itr;	        
	    }
    }
    
    int nBestScore = SCORE_BURST;
    for( int iAce = 0 ; iAce <= nAce ; iAce++) {
        int nScore = nScoreSumExpectAce + (iAce * 1) + ((nAce-iAce)*11);
        if( nScore <= BLACK_JACK && nScore > nBestScore ) {
            nBestScore = nScore;
        }
    }
    return nBestScore;
}

// 現在のスコアの勝率計算.
float CPaizaJacker::GetWinRate( int nScore, const float* pfParentScoreRate ) const
{
    float fRet = 0.0f;
    for( int i = 0 ; i <= BLACK_JACK ; i++ ) {
        if( nScore > i ) {// 同点は親の勝ち.
            fRet += pfParentScoreRate[i];
        }
    }
    return min( fRet, 1.0f );
}

// 親のスコア率計算.
// out_pfParentScoreRateはBLACK_JACK+1個の配列.
void CPaizaJacker::GetParentScoreRate( float* out_pfParentScoreRate, const vector<int>& vecMyCards, const vector<int>& vecYourCards, const vector<int>& vecRemainCards ) const
{
	for( int i = 0 ; i <= BLACK_JACK ; i++ ) {
		out_pfParentScoreRate[ i ] = .0f;
	}

	// 相手のカードがなければデフォルト.
	if( vecYourCards.size() == 0 ) {
		memcpy( out_pfParentScoreRate, DEFAULT_PARENT_SCORE_RATE, sizeof(DEFAULT_PARENT_SCORE_RATE) );
		return;
	}

	int nCardRemain = vecRemainCards.size();
	if( nCardRemain == 0 ) {
		int nScore = GetScore( vecYourCards );
		if( nScore == SCORE_INVALID ) {
			// スコア不明なのでデフォルト.
			memcpy( out_pfParentScoreRate, DEFAULT_PARENT_SCORE_RATE, sizeof(DEFAULT_PARENT_SCORE_RATE) );
		}
		else {
			out_pfParentScoreRate[ nScore ] = 1.0f;
		}
		return;
	}

	for(int iCard = 1 ; iCard <= CARD_NUM_MAX ; iCard++ ) {
		vector<int> vecTmpCards = vecYourCards;
		int nCardCount = 1;

		vector<int> vecTmpRemainCards = vecRemainCards;
		auto itrFind = find( vecTmpRemainCards.begin(), vecTmpRemainCards.end(), iCard );
		if( itrFind == vecTmpRemainCards.end() ) {
			// そのカードは残っていない.
			continue;
		}
		nCardCount = count( vecTmpRemainCards.begin(), vecTmpRemainCards.end(), iCard );
		vecTmpRemainCards.erase( itrFind );// 残りカードから減らす.

		vecTmpCards.push_back( iCard );
		int nScore = GetScore( vecTmpCards );
		if( IsParentHit( vecMyCards, vecYourCards, vecTmpRemainCards ) ) {
			// もう1枚引く.
			float fNextParentScoreRate[BLACK_JACK+1] = { 0 };
			GetParentScoreRate( fNextParentScoreRate, vecMyCards, vecTmpCards, vecTmpRemainCards );
			// もう1枚引いた時のスコア加算.
			for(int iNextScore = 0 ; iNextScore < BLACK_JACK+1 ; iNextScore++ ) {
				out_pfParentScoreRate[ iNextScore ] += fNextParentScoreRate[ iNextScore ] * (float)nCardCount / nCardRemain;
			}
		}
		else if( nScore == SCORE_INVALID ) {
			// スコア不明なのでデフォルト使用.
			for( int i = 0 ; i <= BLACK_JACK ; i++ ) {
				out_pfParentScoreRate[ i ] += DEFAULT_PARENT_SCORE_RATE[ i ] * (float)nCardCount / nCardRemain;
			}
		}
		else {
			out_pfParentScoreRate[ nScore ] += (float)nCardCount / nCardRemain;
		}
	}
}

// 親のカードが見えない時の引いた際のメリットレートを計算.
// return 0.0-1.0
float CPaizaJacker::GetHitWinRate( const vector<int> &vecCards, const float* pfParentScoreRate, const vector<int>& vecRemainCards ) const
{
	if( vecRemainCards.size() == 0 ) {
		// 計算できない.
		return 0.0f;
	}
    float fRet = 0.0f;
    for(int iCard = 1 ; iCard <= CARD_NUM_MAX ; iCard++ ) {
		auto itrFind = find( vecRemainCards.begin(), vecRemainCards.end(), iCard );
		if( itrFind == vecRemainCards.end() ) {
			// そのカードは残っていない.
			continue;
		}
        vector<int> vecTmpCards = vecCards;
        vecTmpCards.push_back( iCard );
        // 引いた後のスコアと勝率.
        int nScore = GetScore( vecTmpCards );
        float fWinRate = 0.0f;
		// 親の期待値から勝率予測.
		fWinRate = GetWinRate( nScore, pfParentScoreRate );
        // 期待値を合計する.
		int nCardCount = count( vecRemainCards.begin(), vecRemainCards.end(), iCard );
		fRet += fWinRate * nCardCount / (float)vecRemainCards.size();
    }
    return min( fRet, 1.0f );
}

/*************************************************/
/* 山札が分かり切っているとき用                  */
/*************************************************/
class CPaizaJacker_Deck : public CPaizaJacker {
public:
	enum WIN_ROUTE {
		eEXIST_HIT_WIN,		// HITで勝てる方法がある.
		eEXIST_STAND_WIN,	// STANDで勝てる方法がある.
		eNO_WIN,			// 勝てる方法がない.
		eNO_CARD,			// カードが足りない(実際に起きる).
	};
public:

	virtual bool IsHitBetter() const;
protected:
	virtual WIN_ROUTE GetWinRoute(const vector<int>& vecMyCards, const vector<int>& vecYourCards, const vector<int>& vecRemainCards, bool bMyStand, bool bYourStand) const;
};
bool CPaizaJacker_Deck::IsHitBetter() const
{
	int nMyScore = GetScore( m_vecMyCard );
	int nYourScore = GetScore( m_vecYourCard );
	if( nYourScore == BLACK_JACK || nMyScore == BLACK_JACK ) {
		// 自分がブラックジャックor相手がブラックジャックなら引かない.
		return false;
	}
	else if( nYourScore == SCORE_BURST ) {
		// 相手がバーストしているなら引かない.
		return false;
	}
	else {
		WIN_ROUTE eWinRoute = GetWinRoute( m_vecMyCard, m_vecYourCard, m_vecRemainCard, false, false );
		if( eWinRoute == eEXIST_HIT_WIN ) {
			return true;
		}
		else if( eWinRoute == eEXIST_STAND_WIN ) {
			return false;
		}
		else if( eWinRoute == eNO_CARD ) {
			// カードがない場合、適当に確率計算する.
			return CPaizaJacker::IsHitBetter();
		}
		else {
			// 勝てそうにない場合適当に頑張る.
			vector<int> vecTmpMyCardsHit0 = m_vecMyCard;
			vector<int> vecTmpMyCardsHit1 = m_vecMyCard;
			if( m_vecRemainCard.size() >= 2 ) {
				vecTmpMyCardsHit0.push_back( m_vecRemainCard[0] );
				vecTmpMyCardsHit1.push_back( m_vecRemainCard[1] );
				int nScoreHit0 = GetScore( vecTmpMyCardsHit0 );
				int nScoreHit1 = GetScore( vecTmpMyCardsHit1 );
				if( nScoreHit0 == SCORE_BURST || nScoreHit1 == SCORE_BURST ) {
					// バーストする可能性があったらSTANDにしておく.
					return false;
				}
				else {
					return true;
				}
			}
			// カード全然ないから適当.
			return true;
		}
	}
}

// HITすることで勝てる道が存在するか.
CPaizaJacker_Deck::WIN_ROUTE CPaizaJacker_Deck::GetWinRoute( const vector<int>& vecMyCards, const vector<int>& vecYourCards, const vector<int>& vecRemainCards, bool bMyStand, bool bYourStand ) const
{
	vector<int> vecTmpMyCards = vecMyCards;
	vector<int> vecTmpYourCards = vecYourCards;
	vector<int> vecTmpRemainCards = vecRemainCards;

	int nMyScore = GetScore( vecTmpMyCards );
	int nYourScore = GetScore( vecTmpYourCards );
	bool bCanPick = true;

	// 確実に勝てない.
	if( nMyScore == SCORE_BURST || nYourScore == BLACK_JACK ) {
		return eNO_WIN;
	}
	
	// もう勝ってる.
	if( nYourScore == SCORE_BURST ) {
		return eEXIST_STAND_WIN;
	}

	// 今STANDすれば相手は引けずに勝てる.
	if( bYourStand && nYourScore < nMyScore ) {
		return eEXIST_STAND_WIN;
	}

	if( bMyStand && bYourStand ) {
		// 両方STANDしてる.
		if( nYourScore < nMyScore ) {
			return eEXIST_STAND_WIN;// 何も選択できず勝てる.
		}
		else {
			return eNO_WIN;
		}
	}


	if( IsParentHit( vecTmpMyCards, vecTmpYourCards, vecTmpRemainCards ) ) {
		if( vecTmpRemainCards.size() == 0 ) {
			return eNO_CARD;
		}
		vecTmpYourCards.push_back( *vecTmpRemainCards.begin() );
		vecTmpRemainCards.erase( vecTmpRemainCards.begin() );
	}
	else {
		bYourStand = true;
	}

	if( bMyStand && bYourStand ) {
		// 両方STANDしたら引けない.
		bCanPick = false;
	}

	int nNewYourScore = GetScore( vecTmpYourCards );
	if ( bCanPick ){
		// 引ける.
		if( vecTmpRemainCards.size() == 0 ) {
			return eNO_CARD;
		}
		if( nNewYourScore == SCORE_BURST ) {
			// 親はバーストするので、HITすべきでない.
			return eEXIST_STAND_WIN;
		}

		// 自分が引いた場合.
		vector<int> vecNextMyCards = vecTmpMyCards;
		vector<int> vecNextRemainCards = vecTmpRemainCards;
		vecNextMyCards.push_back( *vecNextRemainCards.begin() );
		vecNextRemainCards.erase( vecNextRemainCards.begin() );
		WIN_ROUTE eHitWinRoute = GetWinRoute( vecNextMyCards, vecTmpYourCards, vecNextRemainCards, bMyStand || false, bYourStand );
		if( eHitWinRoute == eNO_CARD ) {
			return eNO_CARD;
		}
		if( eHitWinRoute != eNO_WIN ) {
			// 今引けばいずれ勝てる.
			return eEXIST_HIT_WIN;
		}
		// 自分が引かない場合.
		WIN_ROUTE eStandWinRoute = GetWinRoute( vecTmpMyCards, vecTmpYourCards, vecTmpRemainCards, true, bYourStand );
		if( eStandWinRoute == eNO_CARD ) {
			return eNO_CARD;
		}
		if( eStandWinRoute != eNO_WIN ) {
			// 今引かなければいずれ勝てる.
			return eEXIST_STAND_WIN;
		}
		return eNO_WIN;
	}
	else {
		// 引けない.

		// 自分が引かない場合.
		WIN_ROUTE eStandWinRoute = GetWinRoute( vecTmpMyCards, vecTmpYourCards, vecTmpRemainCards, true, bYourStand );
		if( eStandWinRoute == eNO_CARD ) {
			return eNO_CARD;
		}
		if( eStandWinRoute != eNO_WIN ) {
			// 今引かなければいずれ勝てる.
			return eEXIST_STAND_WIN;
		}
		return eNO_WIN;
	}
}

/*************************************************/
/* 緑川赤バニー用                                */
/*************************************************/
class CPaizaJacker_MidorikawaRed : public CPaizaJacker_Deck 
{
protected:
	virtual	long long GetBaseBetMax() const { return 10000; }// 自動的にクリップされるのであまり気にしないでOK.
	virtual	long long GetBetMax() const { return 10 * MILLION; }// 自動的にクリップされるのであまり気にしないでOK.
	virtual bool IsParentHit(const vector<int>& vecMyCards, const vector<int>& vecYourCards, const vector<int>& vecRemainCards) const;
};
// 親がヒットするか.
bool CPaizaJacker_MidorikawaRed::IsParentHit(const vector<int>& vecMyCards, const vector<int>& vecYourCards, const vector<int>& vecRemainCards) const
{
	// 相手のカードを見れるようになるとこんな動きをしていると思われる.
	int nMyScore = GetScore( vecMyCards );
	int nYourScore = GetScore( vecYourCards );
	if( ( nYourScore < GetParentStandScore() || nYourScore < nMyScore ) && nYourScore != SCORE_BURST && nYourScore != BLACK_JACK ) {
		return true;
	}
	return false;
}

/*************************************************/
/* 六村赤バニー用                                */
/*************************************************/
class CPaizaJacker_RokumuraRed : public CPaizaJacker_Deck 
{
public:
	virtual long long GetBet() const;
protected:
	virtual	long long GetBaseBetMax() const { return 40 * MILLION; }// 自動的にクリップされるのであまり気にしないでOK.
	virtual	long long GetBetMax() const { return 10 * BILLION; }// 自動的にクリップされるのであまり気にしないでOK.
	virtual bool IsParentHit(const vector<int>& vecMyCards, const vector<int>& vecYourCards, const vector<int>& vecRemainCards) const;
};
// 親がヒットするか.
bool CPaizaJacker_RokumuraRed::IsParentHit(const vector<int>& vecMyCards, const vector<int>& vecYourCards, const vector<int>& vecRemainCards) const
{
	// 相手のカードを見れるようになるとこんな動きをしていると思われる.
	int nMyScore = GetScore( vecMyCards );
	int nYourScore = GetScore( vecYourCards );
	if( nYourScore >= GetParentStandScore() && nYourScore >= nMyScore ) {
		// 17以上で勝っている→引かない.
		return false;
	}

	if( nYourScore < nMyScore ) {
		if( vecRemainCards.size() >= 1 ) {
			vector<int> vecNextYourCards = vecYourCards;
			vecNextYourCards.push_back( *vecRemainCards.begin() );
			int nNextYourScore = GetScore( vecNextYourCards );
			// 負けていてバーストしない→引く.
			if( nNextYourScore != SCORE_BURST ) {
				return true;
			}
		}
	}
	if( nYourScore < GetParentStandScore() && nYourScore >= nMyScore ) {
		// 17未満で負けている→引く.
		return true;
	}

	return false;
}

long long CPaizaJacker_RokumuraRed::GetBet() const
{
	const int nWinCount = 3; // 3連勝だけを狙う.
	long long llBet = 1LL;

	float fMarginRate = BET_MARGIN_RATE * pow( 1.0f / BET_CALC_WIN_RATE, nWinCount );

	return min( (long long)((float)(m_llMyChip - m_llMaxBetCount) / fMarginRate), GetBetMax()  );
}


// 実際に使用するクラス.
#define JACKER_CLASS CPaizaJacker_RokumuraRed

int main(void)
{
	JACKER_CLASS* pPaizaJacker = new JACKER_CLASS();

	// 入力チェック.
	pPaizaJacker->AnalyzeInput( cin );

	if( pPaizaJacker->IsBet() ) {
		long long llBet = pPaizaJacker->GetBet();
		cout << to_string(llBet) << endl; // 賭けチップ数
	}
	else {
		if( pPaizaJacker->IsHitBetter() ) {
			cout << "HIT" << endl; // カード引く.
		}
		else {
			cout << "STAND" << endl; // 勝負.
		}
	}
	delete pPaizaJacker;
}
