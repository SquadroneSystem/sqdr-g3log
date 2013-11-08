
#include <gtest/gtest.h>
#include <iostream>
#include "testing_helpers.h"
#include "g2log.hpp"
#include "g2logworker.hpp"
#include "std2_make_unique.hpp"
#include <fstream>

using namespace std;
using namespace g2;

namespace testing_helpers {

   std::string g_mockFatal_message = {};
   int g_mockFatal_signal = -1;
   bool g_mockFatalWasCalled = false;


   std::string mockFatalMessage() {
      return g_mockFatal_message;
   }


   int mockFatalSignal() {
      return g_mockFatal_signal;
   }


   bool mockFatalWasCalled() {
      return g_mockFatalWasCalled;
   }


   void mockFatalCall(const g2::FatalMessage& fatal_message) {
      g_mockFatal_message = fatal_message.toString();
      g_mockFatal_signal = fatal_message.signal_id_;
      g_mockFatalWasCalled = true;
      g2::internal::saveMessage(fatal_message.copyToLogMessage());
   }


   void clearMockFatal() {
      g_mockFatal_message = {};
      g_mockFatal_signal = -1;
      g_mockFatalWasCalled = false;
   }


   bool removeFile(std::string path_to_file) {
      return (0 == std::remove(path_to_file.c_str()));
   }


   bool verifyContent(const std::string &total_text, std::string msg_to_find) {
      std::string content(total_text);
      size_t location = content.find(msg_to_find);
      return (location != std::string::npos);
   }


   std::string readFileToText(std::string filename) {
      std::ifstream in;
      in.open(filename.c_str(), std::ios_base::in);
      if (!in.is_open()) {
         return
         {
         }; // error just return empty string - test will 'fault'
      }
      std::ostringstream oss;
      oss << in.rdbuf();
      return oss.str();
   }


   size_t LogFileCleaner::size() {
      return logs_to_clean_.size();
   }


   LogFileCleaner::~LogFileCleaner() {
      std::lock_guard<std::mutex> lock(g_mutex);
      {
         for (const auto& file : logs_to_clean_) {
            if (!removeFile(file)) {
               ADD_FAILURE() << "UNABLE to remove: " << file << std::endl;
            }
         }
         logs_to_clean_.clear();
      } // mutex
   }


   void LogFileCleaner::addLogToClean(std::string path_to_log) {
      std::lock_guard<std::mutex> lock(g_mutex);
      {
         if (std::find(logs_to_clean_.begin(), logs_to_clean_.end(), path_to_log.c_str()) == logs_to_clean_.end())
            logs_to_clean_.push_back(path_to_log);
      }
   }


   ScopedLogger::ScopedLogger() : _currentWorker(g2LogWorker::createWithNoSink()) { }
   ScopedLogger::~ScopedLogger() { }
   g2LogWorker* ScopedLogger::get() { return _currentWorker.get();  }


   RestoreFileLogger::RestoreFileLogger(std::string directory)
   : _scope(new ScopedLogger), _handle(_scope->get()->addSink(std2::make_unique<g2::FileSink>("UNIT_TEST_LOGGER", directory), &g2::FileSink::fileWrite)) {
      using namespace g2;
      g2::initializeLogging(_scope->_currentWorker.get());
      clearMockFatal();
      internal::changeFatalInitHandlerForUnitTesting(&mockFatalCall);

      auto filename = _handle->call(&FileSink::fileName);
      if (!filename.valid()) ADD_FAILURE();
      _log_file = filename.get();
   }


   RestoreFileLogger::~RestoreFileLogger() {
      g2::internal::shutDownLogging();

      if (!removeFile(_log_file))
         ADD_FAILURE();
   }


   // Beware of race between LOG(...) and this function. 
   // since LOG(...) passes two queues but the handle::call only passes one queue 
   // the handle::call can happen faster
   std::string RestoreFileLogger::resetAndRetrieveContent() {
      std::future<std::string> filename = _handle->call(&g2::FileSink::fileName);
      reset(); // flush all queues to sinks
      EXPECT_TRUE(filename.valid());
      auto file = filename.get();
      return readFileToText(file);
   }








} // testing_helpers
