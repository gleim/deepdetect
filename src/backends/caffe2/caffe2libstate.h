/**
 * DeepDetect
 * Copyright (c) 2018 Jolibrain
 * Author: Julien Chicha
 *
 * This file is part of deepdetect.
 *
 * deepdetect is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * deepdetect is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with deepdetect.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CAFFE2LIBSTATE_H
#define CAFFE2LIBSTATE_H

#define REGISTER_CONFIG(type, name, def)				\
									\
private:								\
									\
 type _##name##_default = def;						\
 type _##name##_current = def;						\
 type _##name##_last;							\
 bool name##_changed() const {						\
   return _##name##_last != _##name##_current;				\
 }									\
 void name##_backup() {							\
   _##name##_last = _##name##_current;					\
 }									\
 void name##_reset() {							\
   _##name##_current = _##name##_default;				\
 }									\
 void init_##name() {							\
   _changed.push_back(&Caffe2LibState::name##_changed);			\
   _backup.push_back(&Caffe2LibState::name##_backup);			\
   _reset.push_back(&Caffe2LibState::name##_reset);			\
 }									\
									\
public:						                        \
									\
 const type &name() const {						\
   return _##name##_current;						\
 }									\
 void set_##name(const type &value) {					\
   _##name##_current = value;						\
 }									\
 void set_default_##name(const type &value) {				\
   _##name##_default = value;						\
 }

namespace dd {

  /**
   * \brief Contains the current configuration of the nets :
   *           - training or prediction
   *           - cpu or gpu
   *           - gpu ids
   *           - layer to extract
   *        Each one of these has a getter and two setter (current value, and default value).
   *        The goals of this class are to allow a simple flag management,
   *        and to provide a quick way to know if the nets need
   *        to be reconfigured between two api call.
   */
  class Caffe2LibState {

  private:

    bool _force_init = true;
    std::vector<bool(Caffe2LibState::*)()const> _changed;
    std::vector<void(Caffe2LibState::*)()> _backup;
    std::vector<void(Caffe2LibState::*)()> _reset;

    // Declare here every parameter needed to configure a net
    // (type, name, default value)
    REGISTER_CONFIG(bool, is_gpu, false);
    REGISTER_CONFIG(bool, is_training, false);
    REGISTER_CONFIG(std::vector<int>, gpu_ids, {0});
    REGISTER_CONFIG(std::string, extract_layer, "");

  public:

    // For every declared parameter call the corresponding init function
    Caffe2LibState() {
      init_is_gpu();
      init_is_training();
      init_gpu_ids();
      init_extract_layer();
    }

    /**
     * \brief Is the current configuration different from the last backuped one ?
     * @return True they differ and False otherwise
     */
    bool changed() const {
      bool b = _force_init;
      for (auto f : _changed) {
	b |= (this->*f)();
      }
      return b;
    }

    /**
     * \brief Put the configuration in an 'Uninitialized' state
     *        until the backup() function is called.
     *        Should be used when the nets are in an unstable state
     *        (e.g. an unexpected error occured during the initialization)
     *        and they need to be reconfigured before being used.
     */
    void force_init() {
      _force_init = true;
    }

    /**
     * \brief Tag the current configuration as being the last configuration used.
     */
    void backup() {
      _force_init = false;
      for (auto f : _backup) {
	(this->*f)();
      }
    }

    /**
     * \brief Set current configuration to the default one.
     */
    void reset() {
      for (auto f : _reset) {
	(this->*f)();
      }
    }

  };
}

#undef REGISTER_CONFIG

#endif
