/*
 * Copyright © 2012 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Alan Griffiths <alan@octopull.co.uk>
 */


#ifndef MIR_GRAPHICS_DISPLAY_LISTENER_H_
#define MIR_GRAPHICS_DISPLAY_LISTENER_H_

namespace mir
{
namespace graphics
{

class DisplayListener
{
public:

    virtual void report_successful_setup_of_native_resources() = 0;
    virtual void report_successful_egl_make_current_on_construction() = 0;
    virtual void report_successful_egl_buffer_swap_on_construction() = 0;
    virtual void report_successful_drm_mode_set_crtc_on_construction() = 0;
    virtual void report_successful_display_construction() = 0;
    virtual void report_page_flip_timeout() = 0;

protected:
    DisplayListener() = default;
    ~DisplayListener() = default;
    DisplayListener(const DisplayListener&) = delete;
    DisplayListener& operator=(const DisplayListener&) = delete;
};

}
}

#endif /* MIR_GRAPHICS_DISPLAY_LISTENER_H_ */
