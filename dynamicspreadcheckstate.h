#pragma once


#include <boost/archive/text_oarchive.hpp>

#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/unordered_set.hpp>
namespace ben_tol_lib
{
    struct spread_avg_param
    {
        spread_avg_param(ben_uint32 i_uDate = 0.0f, ben_uint32 i_uTime = 0, double i_dSpread = 0.0f);
        double m_dSpread;
        ben_uint32 m_uDate;
        ben_uint32 m_uTime;
    };



    class spread_exception_table
    {
    public:
        void updateContCntAfterDeleteEntry(const std::string &i_delEntryContName);
        void updateContCntAfterAddEntry(const std::string &i_addEntryContName);
        void delEntry(const std::string &i_delEntryContName, const bool i_isOldest, spread_avg_param &o_delparam);
        unsigned int getDiffContNums();
        void clearContCnt();
        void clearContCntAndETList();
        int getContNums(const std::string &i_cont);
    public:
        std::list<std::pair<std::string, spread_avg_param>> etList_; //contributor,spread
    private:
    public: //for boost serialization , temporary solution ,Wind.
        std::map<std::string, int> contCnt_;    //statistic info of contributor
    };
    //


    class log_tool;

    class dynamic_spreadcheck_state
    {
        //    private :
        //        friend class boost::serialization::access;
    public:
        dynamic_spreadcheck_state();
        typedef enum
        {
            STATIC_LIMIT_TYPE,
            PERCENT_LIMIT_TYPE,
        } SreadCheckTypeEnum;
        typedef struct
        {
            double vSAvg_; //Spread Average from all spreads(regardless passed or failed).
            double mSumSpread_; //sum of all spreads(regardless passed or failed)
            double mSumSpreadPow_;
            ben_uint64 spreadNums_; //spread total conts;
            const spread_exception_table *ptableET_; //Exception Table for the spread check.
            ben_uint64 passSpreadNums_; //spread Passed nums.
            double vSDTV_; //Sample Deviation for all spreads since last calculation.
            double vSMSL_; //Small Move Spread Limitation.
            double vLMSL_; //Large Move Spread Limitation.
            double vSPR_;   //Spread Pass Rate.
            double vUpLimSMS_; //Up limit of the Small Move Spread.
            double vUpLimLMS_; // Up lImeit of the Large Move Spread.
            ben_uint64 vTempMinContSM_;//Template Min Cont SM in Converage for adjusting;
            ben_uint64 vTempMaxContLM_;//Template Max Cont SM in Converage for adjusting;
            ben_uint64 vSMMAccepted_;  //count the SMM accepted
            ben_uint64 vLMMAccepted_;  //count  the LMM accepted
            ben_uint64 vSMMRejected_;  //count the SMM rejected
            ben_uint64 vLMMRejected_;  //count the LMM rejected
            double currentConfigSpreadLimit_;//current static spread;
            double currentSpreadLimit_;
            ben_uint64 mspreadContNums_;
            bool vSMMAccept_;
            bool vLMMAccept_;
        } MemberObservorStruct;
        dynamic_spreadcheck_state(const double i_spreadLimit, log_tool *i_logTool = 0);
        bool getETLogInformation(std::string &o_ETinfo);
        double getCurrentDynSpreadLimit();
        void setCurrentDynSpreadLimit(const double i_spreadLimit);
        void setStatisticInfo(ben_uint64 i_vSMMAccepted, ben_uint64 i_vSMMRejected, ben_uint64 i_vLMMAccepted, ben_uint64 i_vLMMRejected);
        void clearStatisticInfo();
        void clearState();  //when receive any configure changed in dynamic spread check, reuse it for
        bool getAllMemberValue(MemberObservorStruct &o_struct); //serialize,show all the state to VAD
        void recordCheckResult(SreadCheckTypeEnum i_type, const double i_currentSpread, const std::string &i_contributor, ben_uint32 i_spreadDate, ben_uint32 i_spreadTime, bool i_isPass, const CSpreadSetting &i_SpreadSetting);//record the spread check result

        //
        //note we rename the isTriggerDynamicSpreadCheck to GetDynamicSpreadCheckLimit for more readable.  May 9,2016
        //the return bool meaning: if the limit is changed or not.
        //
        bool GetDynamicSpreadCheckLimit(SreadCheckTypeEnum/*i_type*/, const double i_currentSpread, const std::string &i_contributor, const CSpreadSetting &i_SpreadSetting, const boost::posix_time::ptime &i_currentTime);
        //        bool isTriggerDynamicSpreadCheck(SreadCheckTypeEnum/*i_type*/, const double i_currentSpread, const std::string &i_contributor, const CSpreadSetting &i_SpreadSetting, const boost::posix_time::ptime& i_currentTime);
    private:
        bool isTriggeredAfterCheckTableET(const double i_currentSpread, const CSpreadSetting &i_spreadSetting, const boost::posix_time::ptime &i_currentTime);
        bool getSpreadLimitAfterCheck(const double i_configStaticSpreadLimit, const double i_smallMoveMultiplier, const double i_largeMoveMutiplier, const double i_maxSmallMoveStepWise, const double i_maxLargeMoveStepWise, const double i_maxPassPct, const ben_uint64 i_minContSM, const ben_uint64 i_maxContLM, bool i_isMMRejectedAlert = false, bool i_isMMAcceptAlert = false);
    private:
        void clearHistoryInputs();
        void updateInput(const double i_currentStaOrPctSpread, const CSpreadSetting &i_spreadSetting, const std::string &i_cont);
    private:
        bool calAlgorithmVariables(const double /*i_configStaticSpreadLimit*/, const double i_smallMoveMultiplier, const double i_largeMoveMutiplier, const double i_maxSmallMoveStepWise, const double i_maxLargeMoveStepWise, const ben_uint64 i_minContSM, const ben_uint64 i_maxContLM, const double/*i_maxPassPct*/);
        void checkSMSLAndLMSLValidWhenGreaterCurrentSpreadLimt(std::string *o_errorInfo = nullptr);
        void checkSMSLValidWhenInterveneStaticLimitAndCurrent(const double i_maxPassPct);
        void checkSMSLValidWhenLessStaticLimit(const double i_configStaticLimit, const double i_maxPassPct);
        void checkSMSLInOtherConditon();
        void checkLMSLValid();
    private:
    public: //for non-intrusive boost serialization. temporary solution, Wind.
        const static int MAX_AVERAGE_SPREAD_NEED_SPREAD_NUMS;
        double vSAvg_; //Spread Average from all spreads(regardless passed or failed).
        double mSumSpread_; //sum of all spreads(regardless passed or failed)
        double mSumSpreadPow_;  //Sum of thpread;
        unordered_set<std::string> contSet_;
        ben_uint64 mspreadContNums_;
        ben_uint64 spreadNums_; //spread total conts;
        spread_exception_table tableET_; //Exception Table for the spread check.
        unsigned int passSpreadNums_; //spread Passed nums.
        double vSDTV_; //Sample Deviation for all spreads since last calculation.
        double vSMSL_; //Small Move Spread Limitation.
        double vLMSL_; //Large Move Spread Limitation.
        double vSPR_;   //Spread Pass Rate.
        double vUpLimSMS_; //Up limit of the Small Move Spread.
        double vUpLimLMS_; // Up lImeit of the Large Move Spread.
        ben_uint64 vTempMinContSM_;//Template Min Cont SM in Converage for adjusting;
        ben_uint64 vTempMaxContLM_;//Template Max Cont SM in Converage for adjusting;
        double currentSpreadLimit_;
        //record information
        bool vSMMAccept_;
        bool vLMMAccept_;
        bool vSMMReject_;
        bool vLMMReject_;
        ben_uint64 vSMMAccepted_;
        ben_uint64 vLMMAccepted_;
        ben_uint64 vSMMRejected_;
        ben_uint64 vLMMRejected_;
        double currentConfigSpreadLimit_;
        bool bMMAcceptAlert;
        bool bMMRejectAlert;
    private:
        log_tool *plogTool_;
    };

}
