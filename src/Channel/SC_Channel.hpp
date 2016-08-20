#ifndef SC_CHANNELS_HPP_
#define SC_CHANNELS_HPP_

#ifdef SYSTEMC
#include <vector>
#include <string>
#include <systemc>
#include <tlm>
#include <tlm_utils/simple_target_socket.h>
#include <tlm_utils/simple_initiator_socket.h>

#include "../Tools/Frame_trace/Frame_trace.hpp"
#include "../Tools/MIPP/mipp.h"

template <typename R>
class SC_Channel : public sc_core::sc_module, public Channel_interface<R>
{
	SC_HAS_PROCESS(SC_Channel);
	
public:
	tlm_utils::simple_target_socket   <SC_Channel> socket_in;
	tlm_utils::simple_initiator_socket<SC_Channel> socket_out;

public:
	SC_Channel(const int N, const int n_frames = 1, const sc_core::sc_module_name name = "SC_Channel")
	: sc_module(name), Channel_interface<R>(N, n_frames), 
	  socket_in ("socket_in_SC_Channel"), 
	  socket_out("socket_out_SC_Channel")
	{ 
		socket_in.register_b_transport(this, &SC_Channel::b_transport);
	}

	virtual ~SC_Channel() {};

	virtual void add_noise(const mipp::vector<R>& X_N, mipp::vector<R>& Y_N) = 0;

private:
	void b_transport(tlm::tlm_generic_payload& trans, sc_core::sc_time& t)
	{
		const R* buffer_in = (R*)trans.get_data_ptr();
		int length = trans.get_data_length() / sizeof(R);
		
		mipp::vector<R> X_N(length), Y_N(length);
		std::copy(buffer_in, buffer_in + length, X_N.begin());

		this->add_noise(X_N, Y_N);

		tlm::tlm_generic_payload payload;
		payload.set_data_ptr((unsigned char*)Y_N.data());
		payload.set_data_length(Y_N.size() * sizeof(R));

		sc_core::sc_time zero_time(sc_core::SC_ZERO_TIME);
		socket_out->b_transport(payload, zero_time);
	}
};

template <typename R>
using Channel = SC_Channel<R>;
#else
template <typename R>
using Channel = Channel_interface<R>;
#endif

#endif /* SC_CHANNELS_HPP_ */