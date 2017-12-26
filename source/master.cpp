#include <iostream>
#include <sstream>
#include <istream>
#include <vector>
#include <algorithm> //min, max
#include <string.h> //memcpy

using namespace std;

// ���͂ɑ����A�ǂ̊֐����J�[�h�͂��ׂ�1�I���W���v�Z.

#define CARD_NUM_MAX	(13)    // �J�[�h�̐��l���.
#define CARD_MARK_MAX	(4)     // �J�[�h�̃}�[�N���.
#define BLACK_JACK      (21)	// �ŋ��̃X�R�A.
#define SCORE_BURST		(0)		// �o�[�X�g�����X�R�A.
#define SCORE_INVALID	(-1)	// �X�R�A����.
#define BET_MARGIN_RATE		(3.0f)	// ���Ғl�̉��{�]�T�������ăx�b�g���邩.
#define BET_CALC_WIN_RATE	(0.5f)	// �x�b�g�̌v�Z�Ɏg�������̏���.
#define PERMISSIBLE_ERROR	( 0.00001f )	// ���e�덷.

#define MILLION		( 1000000LL )
#define BILLION		( 1000000000LL )

/*************************************************/
/* ��{�̃u���b�N�W���b�N����N���X              */
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
	virtual	long long GetBaseBetMax() const { return 40 * MILLION; }// �����I�ɃN���b�v�����̂ł��܂�C�ɂ��Ȃ���OK.
	virtual	long long GetBetMax() const { return 10 * BILLION; }// �����I�ɃN���b�v�����̂ł��܂�C�ɂ��Ȃ���OK.
	virtual	float GetBaseRewardRate() const { return 3.0f; }
	virtual bool IsParentHit(const vector<int>& vecMyCards, const vector<int>& vecYourCards, const vector<int>& vecRemainCards) const;

protected:
	int GetScore(const vector<int>& vecCards) const;
	float GetWinRate(int nScore, const float * pfParentScoreRate) const;
	void GetParentScoreRate(float * out_pfParentScoreRate, const vector<int>& vecMyCards, const vector<int>& vecYourCards, const vector<int>& vecRemainCards) const;
	float GetHitWinRate(const vector<int>& vecCards, const float * pfParentScoreRate, const vector<int>& vecRemainCards) const;

protected:
	// �x�b�g�p�p�����[�^.
	bool				m_bIsBet;		// Bet���ǂ���.
	long long			m_llMyChip;		// �����Ă���`�b�v��.

	// �J�[�h�p�p�����[�^.
	vector<int>			m_vecMyCard;	// �����̃J�[�h.
	vector<int>			m_vecYourCard;	// ����̃J�[�h.
	vector<int>			m_vecRemainCard;// �R�̃J�[�h.

	// �x�b�g�A�J�[�h���ʃp�����[�^.
	int				m_nGameCount;		// �Q�[���J�E���g.
	int				m_nWinCount;		// �A���J�E���g.
	long long		m_llMaxBetCount;	// �ő�x�b�g.

	static const float DEFAULT_PARENT_SCORE_RATE[BLACK_JACK+1];
};

// �e�̃X�R�A��(�ʃv���O�����Ŏ��s,�l�̌ܓ��̂�������1.0f�����邯�ǖ��Ȃ�).
const float CPaizaJacker::DEFAULT_PARENT_SCORE_RATE[BLACK_JACK+1] = 
{
	0.282f,
	.0f, .0f, .0f, .0f, .0f, .0f, .0f, .0f, .0f, .0f, .0f, .0f, .0f, .0f, .0f, .0f,/* 1�`16 */
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

	// �����̃J�[�hor�����̃`�b�v.
	vector<long long> vecFirstInput;
	ss.clear();
	getline(istInput, strInput);
	ss << strInput;
	while (getline(ss, buffer, ' ')) {
		vecFirstInput.push_back(stoll(buffer));
	}
	if(vecFirstInput[0] == 0LL) {
		// �x�b�g������.
		m_bIsBet = true;
		m_llMyChip = vecFirstInput[1];
	}
	else {
		for(auto itr = vecFirstInput.begin(); itr != vecFirstInput.end(); ++itr) {
			m_vecMyCard.push_back( (int)*itr );
		}
	}

	// �Q�[����.
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

	// �A����.
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

	// �ő�x�b�g��.
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

	// �x�b�g�̏ꍇ�͂����܂�.
	if( m_bIsBet ) {
		return;
	}

	// ����̃J�[�h.
	if( !istInput.eof() ) {
		ss.clear();
		getline(istInput, strInput);
		ss << strInput;
		while (getline(ss, buffer, ' ')) {
			m_vecYourCard.push_back(stoi(buffer));
		}
	}

	// �c��̃J�[�h.
	if( !istInput.eof() ) {
		ss.clear();
		getline(istInput, strInput);
		ss << strInput;
		while (getline(ss, buffer, ' ')) {
			m_vecRemainCard.push_back(stoi(buffer));
		}
	}

	if( m_vecRemainCard.size() == 0 ) {
		// ���͂��Ȃ��ꍇ��1�Z�b�g�g���Ă���Ɖ���.
		for(int iCard = 1 ; iCard <= CARD_NUM_MAX ; iCard++ ) {
			// ���ɔz���Ă���J�[�h�͏���.
			int nOwnCard = count( m_vecMyCard.begin(), m_vecMyCard.end(), iCard ) + count( m_vecYourCard.begin(), m_vecYourCard.end(), iCard );
			// �g�����v1�Z�b�g����Ȃ���������Ȃ��̂�0�����ɂ͂��Ȃ�.
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
		// �ő�ʓr��������ꍇ�͏������z�x�b�g�ł���p�^�[��.
		llBet = GetBaseBetMax();
		float fMargin = (float)GetBaseBetMax() * BET_MARGIN_RATE / BET_CALC_WIN_RATE;
		// ����A�����ɑ΂��āA�]�T������`�b�v������Ȃ�q���ėǂ�.
		long long llMyChipBefore = m_llMyChip - m_llMaxBetCount;// ���������̂͌v�Z�ɓ���Ȃ�(�����܂���̂�)
		if( (long long)fMargin < llMyChipBefore ) {
			fMargin /= BET_CALC_WIN_RATE;
			while( (long long)fMargin < llMyChipBefore ) {
				fMargin /= BET_CALC_WIN_RATE;
				llBet = (long long)( (float)llBet * GetBaseRewardRate() );
			}
			return min( min( m_llMaxBetCount, llBet ), GetBetMax() );
		}
	}

	// �ő�x�b�g�����Ȃ��ꍇ�́A�ő�x�b�g���Œ�l�̃p�^�[��.
	// �x�[�X�̍ő�x�b�g�܂Ń`�b�v���Ȃ��ꍇ���������Ōv�Z.
	llBet = min( max( (long long)( (float)m_llMyChip / BET_MARGIN_RATE * BET_CALC_WIN_RATE), 1LL ), GetBaseBetMax() );
	return min( m_llMyChip, llBet );
}

bool CPaizaJacker::IsHitBetter() const
{
	int nMyScore = GetScore( m_vecMyCard );
	int nYourScore = GetScore( m_vecYourCard );
	if( nYourScore == BLACK_JACK || nMyScore == BLACK_JACK ) {
		// �������u���b�N�W���b�Nor���肪�u���b�N�W���b�N�Ȃ�����Ȃ�.
		return false;
	}
	else if( nYourScore == SCORE_BURST ) {
		// ���肪�o�[�X�g���Ă���Ȃ�����Ȃ�.
		return false;
	}
	else if( !IsParentHit( m_vecMyCard, m_vecYourCard, m_vecRemainCard ) ) {
		if( nMyScore <= nYourScore ) {
			// ���肪STAND��𒴂��Ă��āA�����������Ă���ꍇ��HIT���Ȃ��ƕ�����.
			return true;
		}
		else {
			// ���肪STAND��𒴂��Ă��āA�����������Ă���ꍇ��STAND����Ώ��Ă�.
			return false;
		}
	}
	else {
		// ���̃X�R�A�Ə���.
		float fParentScoreRate[BLACK_JACK+1] = { 0 };
		GetParentScoreRate( fParentScoreRate, m_vecMyCard, m_vecYourCard, m_vecRemainCard );
		float fNowWinRate = GetWinRate( nMyScore, fParentScoreRate );
		float fHitWinRate = GetHitWinRate( m_vecMyCard, fParentScoreRate, m_vecRemainCard );
		if( fHitWinRate >= fNowWinRate - PERMISSIBLE_ERROR/*���e�덷*/ ){ // ���m���A�b�v�Ȃ�.
			return true;
		} else {
			return false;
		}
	}
}
// �e���q�b�g���邩.
bool CPaizaJacker::IsParentHit(const vector<int>& vecMyCards, const vector<int>& vecYourCards, const vector<int>& vecRemainCards) const
{
	// ����̃J�[�h�͌����A������17������.
	int nYourScore = GetScore( vecYourCards );
	return nYourScore == SCORE_INVALID || (nYourScore < GetParentStandScore() && nYourScore != SCORE_BURST && nYourScore != BLACK_JACK);
}
// ���̃J�[�h�̃X�R�A���l��.
// �߂�l - �X�R�A�A0�Ȃ�o�[�X�g.
int CPaizaJacker::GetScore( const vector<int> &vecCards ) const
{
	if( vecCards.size() == 0 ) {
		return SCORE_INVALID;
	}
    int nAce = 0;// ACE��.
    int nScoreSumExpectAce = 0;// ACE�ȊO�̍��v�X�R�A.
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

// ���݂̃X�R�A�̏����v�Z.
float CPaizaJacker::GetWinRate( int nScore, const float* pfParentScoreRate ) const
{
    float fRet = 0.0f;
    for( int i = 0 ; i <= BLACK_JACK ; i++ ) {
        if( nScore > i ) {// ���_�͐e�̏���.
            fRet += pfParentScoreRate[i];
        }
    }
    return min( fRet, 1.0f );
}

// �e�̃X�R�A���v�Z.
// out_pfParentScoreRate��BLACK_JACK+1�̔z��.
void CPaizaJacker::GetParentScoreRate( float* out_pfParentScoreRate, const vector<int>& vecMyCards, const vector<int>& vecYourCards, const vector<int>& vecRemainCards ) const
{
	for( int i = 0 ; i <= BLACK_JACK ; i++ ) {
		out_pfParentScoreRate[ i ] = .0f;
	}

	// ����̃J�[�h���Ȃ���΃f�t�H���g.
	if( vecYourCards.size() == 0 ) {
		memcpy( out_pfParentScoreRate, DEFAULT_PARENT_SCORE_RATE, sizeof(DEFAULT_PARENT_SCORE_RATE) );
		return;
	}

	int nCardRemain = vecRemainCards.size();
	if( nCardRemain == 0 ) {
		int nScore = GetScore( vecYourCards );
		if( nScore == SCORE_INVALID ) {
			// �X�R�A�s���Ȃ̂Ńf�t�H���g.
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
			// ���̃J�[�h�͎c���Ă��Ȃ�.
			continue;
		}
		nCardCount = count( vecTmpRemainCards.begin(), vecTmpRemainCards.end(), iCard );
		vecTmpRemainCards.erase( itrFind );// �c��J�[�h���猸�炷.

		vecTmpCards.push_back( iCard );
		int nScore = GetScore( vecTmpCards );
		if( IsParentHit( vecMyCards, vecYourCards, vecTmpRemainCards ) ) {
			// ����1������.
			float fNextParentScoreRate[BLACK_JACK+1] = { 0 };
			GetParentScoreRate( fNextParentScoreRate, vecMyCards, vecTmpCards, vecTmpRemainCards );
			// ����1�����������̃X�R�A���Z.
			for(int iNextScore = 0 ; iNextScore < BLACK_JACK+1 ; iNextScore++ ) {
				out_pfParentScoreRate[ iNextScore ] += fNextParentScoreRate[ iNextScore ] * (float)nCardCount / nCardRemain;
			}
		}
		else if( nScore == SCORE_INVALID ) {
			// �X�R�A�s���Ȃ̂Ńf�t�H���g�g�p.
			for( int i = 0 ; i <= BLACK_JACK ; i++ ) {
				out_pfParentScoreRate[ i ] += DEFAULT_PARENT_SCORE_RATE[ i ] * (float)nCardCount / nCardRemain;
			}
		}
		else {
			out_pfParentScoreRate[ nScore ] += (float)nCardCount / nCardRemain;
		}
	}
}

// �e�̃J�[�h�������Ȃ����̈������ۂ̃����b�g���[�g���v�Z.
// return 0.0-1.0
float CPaizaJacker::GetHitWinRate( const vector<int> &vecCards, const float* pfParentScoreRate, const vector<int>& vecRemainCards ) const
{
	if( vecRemainCards.size() == 0 ) {
		// �v�Z�ł��Ȃ�.
		return 0.0f;
	}
    float fRet = 0.0f;
    for(int iCard = 1 ; iCard <= CARD_NUM_MAX ; iCard++ ) {
		auto itrFind = find( vecRemainCards.begin(), vecRemainCards.end(), iCard );
		if( itrFind == vecRemainCards.end() ) {
			// ���̃J�[�h�͎c���Ă��Ȃ�.
			continue;
		}
        vector<int> vecTmpCards = vecCards;
        vecTmpCards.push_back( iCard );
        // ��������̃X�R�A�Ə���.
        int nScore = GetScore( vecTmpCards );
        float fWinRate = 0.0f;
		// �e�̊��Ғl���珟���\��.
		fWinRate = GetWinRate( nScore, pfParentScoreRate );
        // ���Ғl�����v����.
		int nCardCount = count( vecRemainCards.begin(), vecRemainCards.end(), iCard );
		fRet += fWinRate * nCardCount / (float)vecRemainCards.size();
    }
    return min( fRet, 1.0f );
}

/*************************************************/
/* �R�D��������؂��Ă���Ƃ��p                  */
/*************************************************/
class CPaizaJacker_Deck : public CPaizaJacker {
public:
	enum WIN_ROUTE {
		eEXIST_HIT_WIN,		// HIT�ŏ��Ă���@������.
		eEXIST_STAND_WIN,	// STAND�ŏ��Ă���@������.
		eNO_WIN,			// ���Ă���@���Ȃ�.
		eNO_CARD,			// �J�[�h������Ȃ�(���ۂɋN����).
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
		// �������u���b�N�W���b�Nor���肪�u���b�N�W���b�N�Ȃ�����Ȃ�.
		return false;
	}
	else if( nYourScore == SCORE_BURST ) {
		// ���肪�o�[�X�g���Ă���Ȃ�����Ȃ�.
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
			// �J�[�h���Ȃ��ꍇ�A�K���Ɋm���v�Z����.
			return CPaizaJacker::IsHitBetter();
		}
		else {
			// ���Ă����ɂȂ��ꍇ�K���Ɋ撣��.
			vector<int> vecTmpMyCardsHit0 = m_vecMyCard;
			vector<int> vecTmpMyCardsHit1 = m_vecMyCard;
			if( m_vecRemainCard.size() >= 2 ) {
				vecTmpMyCardsHit0.push_back( m_vecRemainCard[0] );
				vecTmpMyCardsHit1.push_back( m_vecRemainCard[1] );
				int nScoreHit0 = GetScore( vecTmpMyCardsHit0 );
				int nScoreHit1 = GetScore( vecTmpMyCardsHit1 );
				if( nScoreHit0 == SCORE_BURST || nScoreHit1 == SCORE_BURST ) {
					// �o�[�X�g����\������������STAND�ɂ��Ă���.
					return false;
				}
				else {
					return true;
				}
			}
			// �J�[�h�S�R�Ȃ�����K��.
			return true;
		}
	}
}

// HIT���邱�Ƃŏ��Ă铹�����݂��邩.
CPaizaJacker_Deck::WIN_ROUTE CPaizaJacker_Deck::GetWinRoute( const vector<int>& vecMyCards, const vector<int>& vecYourCards, const vector<int>& vecRemainCards, bool bMyStand, bool bYourStand ) const
{
	vector<int> vecTmpMyCards = vecMyCards;
	vector<int> vecTmpYourCards = vecYourCards;
	vector<int> vecTmpRemainCards = vecRemainCards;

	int nMyScore = GetScore( vecTmpMyCards );
	int nYourScore = GetScore( vecTmpYourCards );
	bool bCanPick = true;

	// �m���ɏ��ĂȂ�.
	if( nMyScore == SCORE_BURST || nYourScore == BLACK_JACK ) {
		return eNO_WIN;
	}
	
	// ���������Ă�.
	if( nYourScore == SCORE_BURST ) {
		return eEXIST_STAND_WIN;
	}

	// ��STAND����Α���͈������ɏ��Ă�.
	if( bYourStand && nYourScore < nMyScore ) {
		return eEXIST_STAND_WIN;
	}

	if( bMyStand && bYourStand ) {
		// ����STAND���Ă�.
		if( nYourScore < nMyScore ) {
			return eEXIST_STAND_WIN;// �����I���ł������Ă�.
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
		// ����STAND����������Ȃ�.
		bCanPick = false;
	}

	int nNewYourScore = GetScore( vecTmpYourCards );
	if ( bCanPick ){
		// ������.
		if( vecTmpRemainCards.size() == 0 ) {
			return eNO_CARD;
		}
		if( nNewYourScore == SCORE_BURST ) {
			// �e�̓o�[�X�g����̂ŁAHIT���ׂ��łȂ�.
			return eEXIST_STAND_WIN;
		}

		// �������������ꍇ.
		vector<int> vecNextMyCards = vecTmpMyCards;
		vector<int> vecNextRemainCards = vecTmpRemainCards;
		vecNextMyCards.push_back( *vecNextRemainCards.begin() );
		vecNextRemainCards.erase( vecNextRemainCards.begin() );
		WIN_ROUTE eHitWinRoute = GetWinRoute( vecNextMyCards, vecTmpYourCards, vecNextRemainCards, bMyStand || false, bYourStand );
		if( eHitWinRoute == eNO_CARD ) {
			return eNO_CARD;
		}
		if( eHitWinRoute != eNO_WIN ) {
			// �������΂����ꏟ�Ă�.
			return eEXIST_HIT_WIN;
		}
		// �����������Ȃ��ꍇ.
		WIN_ROUTE eStandWinRoute = GetWinRoute( vecTmpMyCards, vecTmpYourCards, vecTmpRemainCards, true, bYourStand );
		if( eStandWinRoute == eNO_CARD ) {
			return eNO_CARD;
		}
		if( eStandWinRoute != eNO_WIN ) {
			// �������Ȃ���΂����ꏟ�Ă�.
			return eEXIST_STAND_WIN;
		}
		return eNO_WIN;
	}
	else {
		// �����Ȃ�.

		// �����������Ȃ��ꍇ.
		WIN_ROUTE eStandWinRoute = GetWinRoute( vecTmpMyCards, vecTmpYourCards, vecTmpRemainCards, true, bYourStand );
		if( eStandWinRoute == eNO_CARD ) {
			return eNO_CARD;
		}
		if( eStandWinRoute != eNO_WIN ) {
			// �������Ȃ���΂����ꏟ�Ă�.
			return eEXIST_STAND_WIN;
		}
		return eNO_WIN;
	}
}

/*************************************************/
/* �ΐ�ԃo�j�[�p                                */
/*************************************************/
class CPaizaJacker_MidorikawaRed : public CPaizaJacker_Deck 
{
protected:
	virtual	long long GetBaseBetMax() const { return 10000; }// �����I�ɃN���b�v�����̂ł��܂�C�ɂ��Ȃ���OK.
	virtual	long long GetBetMax() const { return 10 * MILLION; }// �����I�ɃN���b�v�����̂ł��܂�C�ɂ��Ȃ���OK.
	virtual bool IsParentHit(const vector<int>& vecMyCards, const vector<int>& vecYourCards, const vector<int>& vecRemainCards) const;
};
// �e���q�b�g���邩.
bool CPaizaJacker_MidorikawaRed::IsParentHit(const vector<int>& vecMyCards, const vector<int>& vecYourCards, const vector<int>& vecRemainCards) const
{
	// ����̃J�[�h�������悤�ɂȂ�Ƃ���ȓ��������Ă���Ǝv����.
	int nMyScore = GetScore( vecMyCards );
	int nYourScore = GetScore( vecYourCards );
	if( ( nYourScore < GetParentStandScore() || nYourScore < nMyScore ) && nYourScore != SCORE_BURST && nYourScore != BLACK_JACK ) {
		return true;
	}
	return false;
}

/*************************************************/
/* �Z���ԃo�j�[�p                                */
/*************************************************/
class CPaizaJacker_RokumuraRed : public CPaizaJacker_Deck 
{
public:
	virtual long long GetBet() const;
protected:
	virtual	long long GetBaseBetMax() const { return 40 * MILLION; }// �����I�ɃN���b�v�����̂ł��܂�C�ɂ��Ȃ���OK.
	virtual	long long GetBetMax() const { return 10 * BILLION; }// �����I�ɃN���b�v�����̂ł��܂�C�ɂ��Ȃ���OK.
	virtual bool IsParentHit(const vector<int>& vecMyCards, const vector<int>& vecYourCards, const vector<int>& vecRemainCards) const;
};
// �e���q�b�g���邩.
bool CPaizaJacker_RokumuraRed::IsParentHit(const vector<int>& vecMyCards, const vector<int>& vecYourCards, const vector<int>& vecRemainCards) const
{
	// ����̃J�[�h�������悤�ɂȂ�Ƃ���ȓ��������Ă���Ǝv����.
	int nMyScore = GetScore( vecMyCards );
	int nYourScore = GetScore( vecYourCards );
	if( nYourScore >= GetParentStandScore() && nYourScore >= nMyScore ) {
		// 17�ȏ�ŏ����Ă��遨�����Ȃ�.
		return false;
	}

	if( nYourScore < nMyScore ) {
		if( vecRemainCards.size() >= 1 ) {
			vector<int> vecNextYourCards = vecYourCards;
			vecNextYourCards.push_back( *vecRemainCards.begin() );
			int nNextYourScore = GetScore( vecNextYourCards );
			// �����Ă��ăo�[�X�g���Ȃ�������.
			if( nNextYourScore != SCORE_BURST ) {
				return true;
			}
		}
	}
	if( nYourScore < GetParentStandScore() && nYourScore >= nMyScore ) {
		// 17�����ŕ����Ă��遨����.
		return true;
	}

	return false;
}

long long CPaizaJacker_RokumuraRed::GetBet() const
{
	const int nWinCount = 3; // 3�A��������_��.
	long long llBet = 1LL;

	float fMarginRate = BET_MARGIN_RATE * pow( 1.0f / BET_CALC_WIN_RATE, nWinCount );

	return min( (long long)((float)(m_llMyChip - m_llMaxBetCount) / fMarginRate), GetBetMax()  );
}


// ���ۂɎg�p����N���X.
#define JACKER_CLASS CPaizaJacker_RokumuraRed

int main(void)
{
	JACKER_CLASS* pPaizaJacker = new JACKER_CLASS();

	// ���̓`�F�b�N.
	pPaizaJacker->AnalyzeInput( cin );

	if( pPaizaJacker->IsBet() ) {
		long long llBet = pPaizaJacker->GetBet();
		cout << to_string(llBet) << endl; // �q���`�b�v��
	}
	else {
		if( pPaizaJacker->IsHitBetter() ) {
			cout << "HIT" << endl; // �J�[�h����.
		}
		else {
			cout << "STAND" << endl; // ����.
		}
	}
	delete pPaizaJacker;
}
