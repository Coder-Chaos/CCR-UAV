/*
 * can_configurator.cpp
 *
 *  Created on: 2017/10/14
 *      Author: anzai
 */

#include <CANDevice/initializer/can_initializer.h>

void CANInitializer::sendData()
{
	return;
}

void CANInitializer::initDevices()
{
	setMessage(CAN::MESSAGEID_RECEIVE_ENUM_REQUEST, CAN::BROADCAST_ID, 0, nullptr);
	sendMessage(1);
	HAL_Delay(200);
	std::sort(neuron_.begin(), neuron_.end());
	for (unsigned int i = 0; i < neuron_.size(); i++) {
		setMessage(CAN::MESSAGEID_RECEIVE_INITIAL_CONFIG_REQUEST, neuron_.at(i).getSlaveId(), 0, nullptr);
		sendMessage(1);
		HAL_Delay(200);
	}
}

void CANInitializer::configDevice(const hydrus::BoardConfigCmd& config_msg)
{
	uint8_t slave_id = static_cast<uint8_t>(config_msg.data[0]);
	switch (config_msg.command) {
		case hydrus::BoardConfigCmd::SET_SLAVE_ID:
		{
			uint8_t new_slave_id = static_cast<uint8_t>(config_msg.data[1]);
			uint8_t send_data[2];
			send_data[0] = CAN::BOARD_CONFIG_SET_SLAVE_ID;
			send_data[1] = new_slave_id;
			setMessage(CAN::MESSAGEID_RECEIVE_BOARD_CONFIG_REQUEST, slave_id, 2, send_data);
			sendMessage(1);
			break;
		}
		case hydrus::BoardConfigCmd::SET_IMU_SEND_FLAG:
		{
			uint8_t imu_send_data_flag = static_cast<uint8_t>(config_msg.data[1]);
			uint8_t send_data[2];
			send_data[0] = CAN::BOARD_CONFIG_SET_IMU_SEND_FLAG;
			send_data[1] = imu_send_data_flag;
			setMessage(CAN::MESSAGEID_RECEIVE_BOARD_CONFIG_REQUEST, slave_id, 2, send_data);
			sendMessage(1);
			break;
		}
		case hydrus::BoardConfigCmd::SET_SERVO_HOMING_OFFSET:
		{
			uint8_t servo_index = static_cast<uint8_t>(config_msg.data[1]);
			int32_t joint_offset = config_msg.data[2];
			uint8_t send_data[6];
			send_data[0] = CAN::BOARD_CONFIG_SET_SERVO_HOMING_OFFSET;
			send_data[1] = servo_index;
			send_data[2] = joint_offset & 0xFF;
			send_data[3] = (joint_offset >> 8) & 0xFF;
			send_data[4] = (joint_offset >> 16) & 0xFF;
			send_data[5] = (joint_offset >> 24) & 0xFF;
			setMessage(CAN::MESSAGEID_RECEIVE_BOARD_CONFIG_REQUEST, slave_id, 6, send_data);
			sendMessage(1);
			break;
		}
		case hydrus::BoardConfigCmd::SET_SERVO_PID_GAIN:
		{
			uint8_t servo_index = static_cast<uint8_t>(config_msg.data[1]);
			int16_t p_gain = static_cast<uint16_t>(config_msg.data[2]);
			int16_t i_gain = static_cast<uint16_t>(config_msg.data[3]);
			int16_t d_gain = static_cast<uint16_t>(config_msg.data[4]);
			uint8_t send_data[8];
			send_data[0] = CAN::BOARD_CONFIG_SET_SERVO_PID_GAIN;
			send_data[1] = servo_index;
			send_data[2] = p_gain & 0xFF;
			send_data[3] = (p_gain >> 8) & 0xFF;
			send_data[4] = i_gain & 0xFF;
			send_data[5] = (i_gain >> 8) & 0xFF;
			send_data[6] = d_gain & 0xFF;
			send_data[7] = (d_gain >> 8) & 0xFF;
			setMessage(CAN::MESSAGEID_RECEIVE_BOARD_CONFIG_REQUEST, slave_id, 8, send_data);
			sendMessage(1);
			break;
		}
		case hydrus::BoardConfigCmd::SET_SERVO_PROFILE_VEL:
		{
			uint8_t servo_index = static_cast<uint8_t>(config_msg.data[1]);
			int16_t profile_vel = static_cast<uint16_t>(config_msg.data[2]);
			uint8_t send_data[4];
			send_data[0] = CAN::BOARD_CONFIG_SET_SERVO_PROFILE_VEL;
			send_data[1] = servo_index;
			send_data[2] = profile_vel & 0xFF;
			send_data[3] = (profile_vel >> 8) & 0xFF;
			setMessage(CAN::MESSAGEID_RECEIVE_BOARD_CONFIG_REQUEST, slave_id, 4, send_data);
			sendMessage(1);
			break;
		}
		case hydrus::BoardConfigCmd::SET_SERVO_SEND_DATA_FLAG:
		{
			uint8_t servo_index = static_cast<uint8_t>(config_msg.data[1]);
			uint8_t servo_send_data_flag = static_cast<uint8_t>(config_msg.data[2]);
			uint8_t send_data[3];
			send_data[0] = CAN::BOARD_CONFIG_SET_SEND_DATA_FLAG;
			send_data[1] = servo_index;
			send_data[2] = servo_send_data_flag & 0xFF;
			setMessage(CAN::MESSAGEID_RECEIVE_BOARD_CONFIG_REQUEST, slave_id, 3, send_data);
			sendMessage(1);
			break;
		}
		case hydrus::BoardConfigCmd::SET_SERVO_CURRENT_LIMIT:
		{
			uint8_t servo_index = static_cast<uint8_t>(config_msg.data[1]);
			int16_t current_limit = static_cast<uint16_t>(config_msg.data[2]);
			uint8_t send_data[4];
			send_data[0] = CAN::BOARD_CONFIG_SET_SERVO_CURRENT_LIMIT;
			send_data[1] = servo_index;
			send_data[2] = current_limit & 0xFF;
			send_data[3] = (current_limit >> 8) & 0xFF;
			setMessage(CAN::MESSAGEID_RECEIVE_BOARD_CONFIG_REQUEST, slave_id, 4, send_data);
			sendMessage(1);
			break;
		}
		default:
			break;
	}
}

void CANInitializer::receiveDataCallback(uint8_t slave_id, uint8_t message_id, uint32_t DLC, uint8_t* data)
{
	switch (message_id) {
	case CAN::MESSAGEID_SEND_ENUM_RESPONSE:
		neuron_.push_back(Neuron(slave_id));
		break;
	case CAN::MESSAGEID_SEND_INITIAL_CONFIG_0:
		{
			auto slave = std::find(neuron_.begin(), neuron_.end(), Neuron(slave_id));
			if (slave == neuron_.end()) return;
			slave->can_motor_ = CANMotor(slave_id);
			slave->can_servo_ = CANServo(slave_id, data[0]);
			slave->can_imu_ = CANIMU(slave_id, (data[1] != 0) ? true : false);
		}
		break;
	case CAN::MESSAGEID_SEND_INITIAL_CONFIG_1:
		{
			auto slave = std::find(neuron_.begin(), neuron_.end(), Neuron(slave_id));
			if (slave == neuron_.end()) return;
			uint8_t servo_index = data[0];
			slave->can_servo_.servo_[servo_index].id_ = data[1];
			slave->can_servo_.servo_[servo_index].p_gain_ = (data[3] << 8) | data[2];
			slave->can_servo_.servo_[servo_index].i_gain_ = (data[5] << 8) | data[4];
			slave->can_servo_.servo_[servo_index].d_gain_ = (data[7] << 8) | data[6];
		}
		break;
	case CAN::MESSAGEID_SEND_INITIAL_CONFIG_2:
		{
			auto slave = std::find(neuron_.begin(), neuron_.end(), Neuron(slave_id));
			if (slave == neuron_.end()) return;
			uint8_t servo_index = data[0];
			slave->can_servo_.servo_[servo_index].present_position_ = (data[2] << 8) | data[1];
			slave->can_servo_.servo_[servo_index].goal_position_ = (data[2] << 8) | data[1];
			slave->can_servo_.servo_[servo_index].profile_velocity_ = (data[4] << 8) | data[3];
			slave->can_servo_.servo_[servo_index].current_limit_ = (data[6] << 8) | data[5];
			slave->can_servo_.servo_[servo_index].send_data_flag_ = data[7];
			slave->can_servo_.servo_[servo_index].torque_enable_ = true;
		}
		break;
	}
}
