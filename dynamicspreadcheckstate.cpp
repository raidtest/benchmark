
#include "dynamicspreadcheckstate.h"

namespace ben_tol_lib
{

    void spread_exception_table::update_cont_cnt_after_add_entry(const std::string &i_addEntryContName)
    {
        auto itor = contCnt_.find(i_addEntryContName);
        if (contCnt_.end() == itor)
        {
            contCnt_.insert(std::pair<std::string, int>(i_addEntryContName, 1));
        }
        else
        {
            itor->second++;
        }

        return;
    }


    void spread_exception_table::update_cont_cnt_after_delete_entry(const std::string &i_addEntryContName)
    {
        auto itor = contCnt_.find(i_addEntryContName);
        if (contCnt_.end() != itor)
        {
            itor->second--;
            if (itor->second <= 0)
            {
                contCnt_.erase(itor);
            }
        }

        return;
    }

    void spread_exception_table::del_entry(const std::string &i_delEntryContName, const bool i_isOldest, spread_avg_param &o_delparam)
    {
        if (i_isOldest == true)
        {
            // for the oldest one is first pushed in the ET table;
            auto iter = etList_.begin();
            for (; iter != etList_.end(); iter++)
            {
                if (i_delEntryContName == iter->first)
                {
                    break;
                }
            }
            if (iter != etList_.end())
            {
                o_delparam = iter->second;
                const std::string delStr = iter->first;
                etList_.erase(iter);
                update_cont_cnt_after_delete_entry(delStr);
            }
        }
        else //else delete the latest one
        {
            //the latest one
            auto riter = etList_.rbegin();
            for (; riter != etList_.rend(); riter++)
            {
                if (i_delEntryContName == riter->first)
                {
                    break;
                }
            }

            if (riter != etList_.rend())
            {
                o_delparam = riter->second;
                const std::string delStr = riter->first;
                etList_.erase((++riter).base());
                update_cont_cnt_after_delete_entry(delStr);
            }
        }
    }


    int spread_exception_table::get_cont_nums(const std::string &i_cont)
    {
        int rt = 0;
        auto iter = contCnt_.find(i_cont);
        if (contCnt_.end() != iter)
        {
            rt = iter->second;
        }

        return rt;
    }


    unsigned int  spread_exception_table::get_diff_cont_nums()
    {
        return contCnt_.size();
    }

    void spread_exception_table::clear_cont_cnt()
    {
        contCnt_.clear();
    }

    void spread_exception_table::clear_cont_cnt_and_ET_list()
    {
        etList_.clear();
        clear_cont_cnt();
    }

    spread_avg_param::spread_avg_param(ben_uint32 i_uDate, ben_uint32 i_uTime, double i_dSpread) :
        m_dSpread(i_dSpread),
        m_uDate(i_uDate),
        m_uTime(i_uTime)
    {
    }

    dynamic_spreadcheck_state::dynamic_spreadcheck_state()
    {

    }

    dynamic_spreadcheck_state::dynamic_spreadcheck_state(const double i_spreadLimit, log_tool *i_logTool) : vSAvg_(0.0f),
        mSumSpread_(0), mSumSpreadPow_(0), contSet_(), mspreadContNums_(0), spreadNums_(0), tableET_(), passSpreadNums_(0),
        vSDTV_(0.0f), vSMSL_(0.0f), vLMSL_(0.0f), vSPR_(0.0f), vUpLimSMS_(0.0f),
        vUpLimLMS_(0.0f), vTempMaxContLM_(0), vTempMinContSM_(0),
        currentSpreadLimit_(i_spreadLimit), vSMMAccept_(false), vLMMAccept_(false), vSMMReject_(false), vLMMReject_(false),
        vSMMAccepted_(0), vLMMAccepted_(0), vSMMRejected_(0), vLMMRejected_(0),
        currentConfigSpreadLimit_(i_spreadLimit), plogTool_(i_logTool),
        bMMAcceptAlert(false), bMMRejectAlert(false)
    {

    }



    bool dynamic_spreadcheck_state::get_ET_log_information(std::string &o_ETinfo)
    {
        int iCnt = 1;
        o_ETinfo += "# of SET=";
        o_ETinfo += std::to_string(static_cast<long long>(tableET_.etList_.size()));
        for (std::list<std::pair<std::string, spread_avg_param>>::iterator iter = tableET_.etList_.begin(); iter != tableET_.etList_.end(); iter++)
        {
            o_ETinfo += ";ET#";
            o_ETinfo += std::to_string(static_cast<long long>(iCnt++));
            o_ETinfo += "=(";
            //o_ETinfo += "Cont=";
            o_ETinfo += iter->first;
            o_ETinfo += " ";
            o_ETinfo += std::to_string(static_cast<long double>(iter->second.m_dSpread));
            o_ETinfo += " ";
            o_ETinfo += std::to_string(static_cast<long long>(iter->second.m_uDate));
            o_ETinfo += "_";
            char buf[50];
            ::sprintf_s(buf, sizeof(buf), "%02d%02d%02d", int(iter->second.m_uTime / 10000), (int)(iter->second.m_uTime % 10000 / 100), iter->second.m_uTime % 100);
            o_ETinfo += std::string(buf);
            o_ETinfo += ")";
        }
        o_ETinfo += " ";
        return true;
    }

    void dynamic_spreadcheck_state::set_current_dynspread_limit(const double i_spreadLimit)
    {
        clear_state();
        currentSpreadLimit_ = i_spreadLimit;
    }

    double dynamic_spreadcheck_state::get_current_dynspread_limit()
    {
        return currentSpreadLimit_;
    }

    void dynamic_spreadcheck_state::clear_state()
    {
        clear_history_inputs();
    }

    bool dynamic_spreadcheck_state::get_all_value(MemberObservorStruct &o_struct)
    {
        o_struct.mSumSpread_ = mSumSpread_;
        o_struct.passSpreadNums_ = passSpreadNums_;
        o_struct.spreadNums_ = spreadNums_;
        o_struct.ptableET_ = &tableET_;
        o_struct.vLMSL_ = vLMSL_;
        o_struct.vSAvg_ = vSAvg_;
        o_struct.vSDTV_ = vSDTV_;
        o_struct.vSMSL_ = vSMSL_;
        o_struct.vSPR_ = vSPR_;
        o_struct.vTempMaxContLM_ = vTempMaxContLM_;
        o_struct.vTempMinContSM_ = vTempMinContSM_;
        o_struct.vUpLimLMS_ = vUpLimLMS_;
        o_struct.vUpLimSMS_ = vUpLimSMS_;
        o_struct.vLMMAccepted_ = vLMMAccepted_;
        o_struct.vLMMRejected_ = vLMMRejected_;
        o_struct.vSMMAccepted_ = vSMMAccepted_;
        o_struct.vSMMRejected_ = vSMMRejected_;
        o_struct.currentConfigSpreadLimit_ = currentConfigSpreadLimit_;
        o_struct.currentSpreadLimit_ = currentSpreadLimit_;
        o_struct.vLMMAccept_ = vLMMAccept_;
        o_struct.vSMMAccept_ = vSMMAccept_;
        o_struct.mspreadContNums_ = mspreadContNums_;
        return true;
    }

    void dynamic_spreadcheck_state::record_check_result(sread_check_type_enum/*i_type*/,
            const double i_currentSpread,
            const std::string &i_contributor,
            ben_uint32 i_spreadDate,
            ben_uint32 i_spreadTime,
            bool i_isPass,
            const CSpreadSetting &i_SpreadSetting)
    {
        if (i_isPass)
        {
            passSpreadNums_++;
        }
        else
        {
            if ((i_SpreadSetting.m_SETPerContLimit > 0) && (tableET_.get_cont_nums(i_contributor) >= i_SpreadSetting.m_SETPerContLimit))
            {
                bool isDelOld = false;
                if (i_SpreadSetting.m_SETOperationOption > 0)isDelOld = true;
                spread_avg_param delParam;
                tableET_.del_entry(i_contributor, isDelOld, delParam);

                //log
                std::stringstream logInfo;
                logInfo << i_contributor << " Spread=" << delParam.m_dSpread;
                logInfo << " TS=" << delParam.m_uDate << "_" << delParam.m_uTime;
                plogTool_->logMyTolMsg("DSC", "----", logInfo.str(), "SETPerCont full. Remove oldest entry.");
            }

            //defaut the latest entry push back.
            tableET_.etList_.push_back(std::pair<std::string, spread_avg_param>(i_contributor, spread_avg_param(i_spreadDate, i_spreadTime, i_currentSpread)));
            tableET_.update_cont_cnt_after_add_entry(i_contributor);
            //log
            std::stringstream logInfo;
            logInfo << "Spread=" << i_currentSpread << ";TS=" << i_spreadDate << " " << GETTIMESTRING(i_spreadTime);
            plogTool_->logMyTolMsg("DSC", i_contributor, logInfo.str(), "ET adds an entry");
        }
    }

    bool dynamic_spreadcheck_state::get_dynamic_spread_check_limit(sread_check_type_enum/*i_type*/, const double i_currentSpread, const std::string &i_contributor, const CSpreadSetting &i_SpreadSetting, const boost::posix_time::ptime &i_currentTime)
    {
        //table ET process
        bool rt_isTrigger = false;

        rt_isTrigger = is_triggered_after_check_table_ET(i_currentSpread, i_SpreadSetting, i_currentTime);

        if (rt_isTrigger)
        {
            get_spread_limit_after_check(i_SpreadSetting.m_dCheckLimit, i_SpreadSetting.m_SmallMoveMultiplier, i_SpreadSetting.m_LargeMoveMultiplier,
                                     i_SpreadSetting.m_MaxSMStepwise, i_SpreadSetting.m_MaxLMStepwise, i_SpreadSetting.m_MaxPassPct, i_SpreadSetting.m_MinContSM, i_SpreadSetting.m_MaxContLM
                                     , i_SpreadSetting.m_bMMRejectAlert, i_SpreadSetting.m_bMMAcceptAlert);

            //clear et table
            //log
            std::string etInfo;
            get_ET_log_information(etInfo);
            plogTool_->logMyTolMsg("DSC", "----", etInfo, "DynSpread triggered. Clear SET.");

            clear_history_inputs();
        }

        //algorithm of dynamic spread check
        update_input(i_currentSpread, i_SpreadSetting, i_contributor);

        return rt_isTrigger;
    }

    //private
    bool dynamic_spreadcheck_state::is_triggered_after_check_table_ET(const double/*i_currentSpread*/, const CSpreadSetting &i_spreadSetting, const boost::posix_time::ptime &i_currentTime)
    {
        bool rt = false;
        if (0 == tableET_.etList_.size())
        {
            return false;
        }
        //ET check.when there are entries are out of time, clear the entries.

        auto itor = tableET_.etList_.begin();
        while (itor != tableET_.etList_.end())
        {

            boost::posix_time::ptime ptParam(boost::gregorian::date(itor->second.m_uDate / 10000, boost::date_time::months_of_year(itor->second.m_uDate % 10000 / 100), itor->second.m_uDate % 100),
                                             boost::posix_time::time_duration(itor->second.m_uTime / 10000, itor->second.m_uTime % 10000 / 100, itor->second.m_uTime % 100));
            boost::posix_time::time_duration tmDur = i_currentTime - ptParam;

            //if time reach the 9999year, there will be a problem.
            if ((0 != i_spreadSetting.m_SETWindow) && (float_greater(tmDur.total_seconds(), i_spreadSetting.m_SETWindow)))
            {
                //log
                std::stringstream logInfo;
                logInfo << " Time=" << itor->second.m_uDate << "_" << GETTIMESTRING(itor->second.m_uTime) << ";Value=" << itor->second.m_dSpread;
                plogTool_->logMyTolMsg("DSC", itor->first, logInfo.str(), "ET Entry TimeOut");

                const std::string delStr = itor->first;
                itor = tableET_.etList_.erase(itor);  //delete the entry
                tableET_.update_cont_cnt_after_delete_entry(delStr);
                //LogTool::logMsg();
            }
            else
            {
                itor++;
            }
        }

        //if ET is full, trigger the dynamic current spread
        if (static_cast<ben_uint64>(i_spreadSetting.m_SETSize) <= tableET_.etList_.size())
        {
            rt = true;
        }

        return rt;
    }

    void dynamic_spreadcheck_state::update_input(const double i_currentStaOrPctSpread, const CSpreadSetting &i_spreadSetting, const std::string &i_cont)
    {
        if (spreadNums_ > MAX_AVERAGE_SPREAD_NEED_SPREAD_NUMS)
        {
            clear_history_inputs();
        }

        spreadNums_++;
        mSumSpread_ += i_currentStaOrPctSpread;
        vSAvg_ = mSumSpread_ / spreadNums_;
        mSumSpreadPow_ += std::pow(i_currentStaOrPctSpread, 2);

        //record the current config static spread limit
        currentConfigSpreadLimit_ = i_spreadSetting.m_dCheckLimit;
        //record contributor
        if (contSet_.end() == contSet_.find(i_cont))
        {
            contSet_.insert(i_cont);
        }
        mspreadContNums_ = contSet_.size();
    }

    void dynamic_spreadcheck_state::clear_history_inputs()
    {
        spreadNums_ = 0;
        mSumSpreadPow_ = 0;
        mSumSpread_ = 0;
        vSAvg_ = 0;
        tableET_.clear_cont_cnt_and_ET_list();
        passSpreadNums_ = 0;
        vSMMAccept_ = false;
        vLMMAccept_ = false;
        vSMMReject_ = false;
        vLMMReject_ = false;
        contSet_.clear();
        mspreadContNums_ = 0;
    }

    void dynamic_spreadcheck_state::clear_statistic_info()
    {
        vSMMAccepted_ = 0;
        vSMMRejected_ = 0;
        vLMMAccepted_ = 0;
        vLMMRejected_ = 0;
    }

    void dynamic_spreadcheck_state::set_statistic_info(ben_uint64 i_vSMMAccepted, ben_uint64 i_vSMMRejected, ben_uint64 i_vLMMAccepted, ben_uint64 i_vLMMRejected)
    {
        vSMMAccepted_ = i_vSMMAccepted;
        vSMMRejected_ = i_vSMMRejected;
        vLMMAccepted_ = i_vLMMAccepted;
        vLMMRejected_ = i_vLMMRejected;
    }

    bool dynamic_spreadcheck_state::get_spread_limit_after_check(const double i_configStaticSpreadLimit, const double i_smallMoveMultiplier, const double i_largeMoveMutiplier
            , const double i_maxSmallMoveStepWise, const double i_maxLargeMoveStepWise, const double i_maxPassPct,
            const ben_uint64 i_minContSM, const ben_uint64 i_maxContLM, bool i_isMMRejectedAlert, bool i_isMMAcceptAlert)
    {

        if (!cal_algorithm_variables(i_configStaticSpreadLimit, i_smallMoveMultiplier, i_largeMoveMutiplier,
                                   i_maxSmallMoveStepWise, i_maxLargeMoveStepWise, i_minContSM, i_maxContLM, i_maxPassPct))
        {
            return false;
        }

        double spreaLimitBefore = currentSpreadLimit_;  //just for alert info
        std::string logInfo = "DynSpread triggered.";
        std::string subErrorInfo;
        if (float_greater(vSMSL_, currentSpreadLimit_))
        {
            check_SMSL_and_LMSL_valid_when_greater_current_spread_limt(&subErrorInfo);
            logInfo += "SMSL>CSL";
        }
        else if (float_greater(vSMSL_, i_configStaticSpreadLimit) && float_greater(currentSpreadLimit_, vSMSL_))
        {
            check_SMSL_valid_when_intervene_static_limit_and_current(i_maxPassPct);
            logInfo += "StaticSpLim<SMSL<CSL";
        }
        else if (float_greater(i_configStaticSpreadLimit, vSMSL_) || FloatEqual(vSMSL_, i_configStaticSpreadLimit))
        {
            if (!FloatEqual(i_configStaticSpreadLimit, currentSpreadLimit_))
            {
                check_SMSL_valid_when_less_static_limit(i_configStaticSpreadLimit, i_maxPassPct);
                logInfo += "SMSL<=StaticSpLim";
            }
            else
            {
                check_SMSL_in_other_conditon();
                logInfo += " But rejected as SMSL<=StaticSpLim";
            }

            ///            checkSMSLValidWhenLessStaticLimit(i_configStaticSpreadLimit, i_maxPassPct);
            //            logInfo += "SMSL<=StaticSpLim";
        }
        else
        {
            //refuse SMSL
            check_SMSL_in_other_conditon();
            logInfo += "other condition reject SMSL";
        }

        //log so much log,you can't image content why they demand so much information,amazing!!!!.
        std::string alertInfo = GETREALLOG(vSMMReject_, "Spread SMM reject", "") + GETREALLOG(vSMMAccept_, "Spread SMM accept", "") + GETREALLOG(vLMMAccept_, "Spread LMM accept", "") + GETREALLOG(vLMMReject_, "Spread LMM reject", "");
        std::string eTInfo;
        get_ET_log_information(eTInfo);
        std::stringstream paraInfo;
        paraInfo << alertInfo << ";NewSpLim=" << currentSpreadLimit_ << ";OldSpLim=" << spreaLimitBefore << ";StaticSpLim=" << i_configStaticSpreadLimit << subErrorInfo;
        paraInfo << ";SMSL=" << vSMSL_ << ";LMSL=" << vLMSL_ << ";CurrentPassRate=" << vSPR_ << ";MaxPassRate=" << static_cast<float>(i_maxPassPct) / 100.0 << ";UplimSMS=" << vUpLimSMS_ << ";UpLimLMS=" << vUpLimLMS_;
        paraInfo << ";MinContSM=" << vTempMinContSM_ << ";MinContLM=" << vTempMaxContLM_ << ";SMMAccept=" << vSMMAccept_ << ";LMMAccept=" << vLMMAccept_ << ";" << eTInfo;
        //the log config to produce alert information
        LogMsgConfig logConfig((i_isMMRejectedAlert && (vSMMReject_ || vLMMReject_)) || (i_isMMAcceptAlert && (vSMMAccept_ || vLMMAccept_)), alertInfo);
        plogTool_->logMyTolMsg(logConfig.getAlertInfo().c_str(), "----", paraInfo.str(), logInfo);
        return true;
    }

    bool dynamic_spreadcheck_state::cal_algorithm_variables(const double /*i_configStaticSpreadLimit*/, const double i_smallMoveMultiplier, const double i_largeMoveMutiplier, const double i_maxSmallMoveStepWise, const double  i_maxLargeMoveStepWise, const ben_uint64 i_minContSM, const ben_uint64 i_maxContLM, const double/*i_maxPassPct*/)
    {
        if (spreadNums_ < 2)
        {
            return false;
        }

        double nsdtvPow = std::fabs(spreadNums_ * vSAvg_ * vSAvg_ + mSumSpreadPow_ - 2 * vSAvg_ * mSumSpread_);
        if (IsFloatZero(nsdtvPow))
        {
            nsdtvPow = 0.0f; //so interesting! waste me so much time to find this problem.
        }
        vSDTV_ = std::sqrt(nsdtvPow / (spreadNums_ - 1));  //(a-b)pow2 = apow2 +bpow2-2ab
        vSMSL_ = vSAvg_ + vSDTV_ * i_smallMoveMultiplier;
        vLMSL_ = vSAvg_ + vSDTV_ * i_largeMoveMutiplier;
        vSPR_ = static_cast<double>(passSpreadNums_) / static_cast<double>(spreadNums_);
        vUpLimSMS_ = currentSpreadLimit_ * (i_maxSmallMoveStepWise + 1);
        vUpLimLMS_ = currentSpreadLimit_ * (i_maxLargeMoveStepWise + 1);

        //update tempMinContSM_,vTempMaxCOntLM_;
        vTempMinContSM_ = i_minContSM;
        vTempMaxContLM_ = i_maxContLM;
        unsigned int contNums = contSet_.size();
        if (contNums <= i_minContSM)
        {
            vTempMinContSM_ = i_minContSM / 2 + 1;
            if (contNums <= 0)
            {
                vTempMaxContLM_ = vTempMinContSM_;
            }
            else
            {
                vTempMaxContLM_ = (vTempMinContSM_ > (contNums - 1)) ? vTempMinContSM_ : (contNums - 1);
            }
        }
        else if ((contNums > i_minContSM) && (contNums <= i_maxContLM))
        {
            //contNums>0 always in this condition.
            vTempMaxContLM_ = (vTempMinContSM_ > (contNums - 1)) ? vTempMinContSM_ : (contNums - 1);
        }
        else {}

        //revise SMSL
        if (float_greater(vSMSL_, vUpLimSMS_) || FloatEqual(vSMSL_, vUpLimSMS_))
        {
            vSMSL_ = vUpLimSMS_;
        }

        return true;
    }

    void dynamic_spreadcheck_state::check_SMSL_and_LMSL_valid_when_greater_current_spread_limt(std::string *o_errorInfo)
    {
        double revisedLMSL = (float_greater(vLMSL_, vUpLimLMS_)) ? vUpLimLMS_ : vLMSL_;

        std::set<std::string> vContGreaterSMSL;
        std::set<std::string> vContGreaterLMSL;

        for (auto itor = tableET_.etList_.begin(); itor != tableET_.etList_.end(); itor++)
        {
            if (float_greater(vSMSL_, itor->second.m_dSpread))
            {
                if (vContGreaterSMSL.end() == vContGreaterSMSL.find(itor->first))
                    vContGreaterSMSL.insert(itor->first);
            }

            if (float_greater(revisedLMSL, itor->second.m_dSpread))
            {
                if (vContGreaterLMSL.end() == vContGreaterLMSL.find(itor->first))
                    vContGreaterLMSL.insert(itor->first);
            }
        }

        vSMMAccept_ = false;
        vLMMAccept_ = false;
        vLMMReject_ = false;
        vSMMReject_ = false;
        //get Spread Limit
        if (vContGreaterSMSL.size() >= vTempMinContSM_)
        {
            currentSpreadLimit_ = vSMSL_;
            vSMMAccept_ = true;
            vSMMAccepted_++;   //tuple the sum accept nums
        }
        else
        {
            vLMSL_ = revisedLMSL;
            if (float_greater(vLMSL_, currentSpreadLimit_))
            {
                if (vContGreaterLMSL.size() >= vTempMaxContLM_)
                {
                    currentSpreadLimit_ = vLMSL_;
                    vLMMAccept_ = true;
                    vLMMAccepted_++;  // tupple the accepted nums
                }
            }

            if (!vLMMAccept_)
            {
                vLMMRejected_++;
                vLMMReject_ = true;
            }
        }

        if (false == vSMMAccept_)
        {
            vSMMRejected_++;
            vSMMReject_ = true;
        }

        if ((false == vLMMAccept_) && (false == vSMMAccept_))
        {
            if (plogTool_)
                plogTool_->logMyTolMsg("DSC", "----", "", "SMM and LMM are both reject please revise the setting!");
        }

        //add the log to errorinfo
        stringstream stllog;
        stllog << ";ActualContSM=" << vContGreaterSMSL.size() << ";ActualContLM=" << vContGreaterLMSL.size();
        if (o_errorInfo)*o_errorInfo += stllog.str();

        return;
    }

    void dynamic_spreadcheck_state::check_SMSL_valid_when_intervene_static_limit_and_current(const double i_maxPassPct)
    {
        vSMMAccept_ = false;
        vLMMAccept_ = false;
        vLMMReject_ = false;
        vSMMReject_ = false;

        if (float_greater(vSPR_, (i_maxPassPct / 100.0f)))
        {
            currentSpreadLimit_ = vSMSL_;
            vSMMAccept_ = true;
            vSMMAccepted_++;
        }

        if (!vSMMAccept_)
        {
            vSMMRejected_++;
            vSMMReject_ = true;
        }

    }

    void dynamic_spreadcheck_state::check_SMSL_valid_when_less_static_limit(const double i_configStaticLimit, const double i_maxPassPct)
    {
        vSMMAccept_ = false;
        vLMMAccept_ = false;
        vLMMReject_ = false;
        vSMMReject_ = false;

        if (float_greater(vSPR_, i_maxPassPct / 100.0f))
        {
            currentSpreadLimit_ = i_configStaticLimit;
            vSMMAccept_ = true;
            vSMMAccepted_++;
        }

        if (!vSMMAccept_)
        {
            vSMMRejected_++;
            vSMMReject_ = true;
        }
    }

    void dynamic_spreadcheck_state::check_SMSL_in_other_conditon()
    {
        vSMMAccept_ = false;
        vLMMAccept_ = false;
        vLMMReject_ = false;
        vSMMReject_ = false;
        if (!vSMMAccept_)
        {
            vSMMRejected_++;
            vSMMReject_ = true;
        }
    }

    void dynamic_spreadcheck_state::check_LMSL_valid()
    {
        vLMMAccept_ = false;
        double revisedLMSL = (float_greater(vLMSL_, vUpLimLMS_)) ? vUpLimLMS_ : vLMSL_;
        std::set<std::string> vContGreaterLMSL;
        for (auto itor = tableET_.etList_.begin(); itor != tableET_.etList_.end(); itor++)
        {
            if (float_greater(revisedLMSL, itor->second.m_dSpread))
            {
                if (vContGreaterLMSL.end() == vContGreaterLMSL.find(itor->first))
                    vContGreaterLMSL.insert(itor->first);
            }
        }

        vLMSL_ = revisedLMSL;
        if (float_greater(vLMSL_, currentSpreadLimit_))
        {
            if (vContGreaterLMSL.size() > vTempMaxContLM_)
            {
                currentSpreadLimit_ = vLMSL_;
                vLMMAccept_ = true;
                vLMMAccepted_++;  // tupple the accepted nums
            }
        }

        if (!vLMMAccept_)
        {
            vLMMRejected_++;
            vLMMReject_ = true;
        }
    }

    const int dynamic_spreadcheck_state::MAX_AVERAGE_SPREAD_NEED_SPREAD_NUMS = std::numeric_limits<int>::max();
}