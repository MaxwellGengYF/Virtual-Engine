#include "JobNode.h"
#include "JobBucket.h"
#include "VectorPool.h"
JobNode::~JobNode()
{
	if (destructorFunc != nullptr)
		destructorFunc(ptr);
	if(dependingEvent && vectorPool)
	dependingEvent->clear();
	vectorPool->Delete(dependingEvent);
}

JobNode* JobNode::Execute(ConcurrentQueue<JobNode*>& taskList, std::condition_variable& cv)
{
	executeFunc(ptr);
	auto ite = dependingEvent->begin();
	JobNode* nextNode = nullptr;
	while(ite != dependingEvent->end())
	{
		JobNode* node = *ite;
		unsigned int dependingCount = --node->targetDepending;
		if (dependingCount == 0)
		{
			nextNode = node;
			++ite;
			break;
		}
		++ite;
	}
	for (; ite != dependingEvent->end(); ++ite)
	{
		JobNode* node = *ite;
		unsigned int dependingCount = --node->targetDepending;
		if (dependingCount == 0)
		{
			taskList.Push(node);
			{
				std::lock_guard<std::mutex> lck(*threadMtx);
				cv.notify_one();
			}
		}
	}
	return nextNode;
}
/*
void JobNode::Precede(JobNode* depending)
{
	depending->targetDepending++;
	dependingEvent->emplace_back(depending);
	if (depending->vecIndex >= 0)
	{
		JobBucket* bucket = depending->bucket;
		auto lastIte = bucket->jobNodesVec.end() - 1;
		auto indIte = bucket->jobNodesVec.begin() + depending->vecIndex;
		*indIte = *lastIte;
		(*indIte)->vecIndex = depending->vecIndex;
		bucket->jobNodesVec.erase(lastIte);
		depending->vecIndex = -1;
	}
}
*/