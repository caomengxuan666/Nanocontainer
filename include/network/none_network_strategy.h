#include "network_strategy.h"

class NoneNetworkStrategy : public NetworkStrategy {
public:
    void setup_in_container() override;
};