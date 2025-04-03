"""

Move to specified pose

Author: Daniel Ingram (daniel-s-ingram)
        Atsushi Sakai (@Atsushi_twi)
        Seied Muhammad Yazdian (@Muhammad-Yazdian)

P. I. Corke, "Robotics, Vision & Control", Springer 2017, ISBN 978-3-319-54413-7

"""

import matplotlib.pyplot as plt
import numpy as np
import random
#import sys
#sys.path.insert(1, '/Users/poonoi/Library/CloudStorage/OneDrive-Personal/Work/AMR/PythonRobotics-master')
#import file

#from utils.angle import angle_mod
import math

def opposite_angle(angle_radians):
    # หามุมตรงข้ามกัน
    opposite_radians = (angle_radians + math.pi) % (2 * math.pi)
    return opposite_radians

def cal_intermediate_point(x,y,theta,dist):     
    x_inter = x + math.cos(theta)*dist
    y_inter = y + math.sin(theta)*dist
    theta_inter = theta
    return x_inter,y_inter,theta_inter

def angle_mod(x, zero_2_2pi=False, degree=False):
    """
    Angle modulo operation
    Default angle modulo range is [-pi, pi)

    Parameters
    ----------
    x : float or array_like
        A angle or an array of angles. This array is flattened for
        the calculation. When an angle is provided, a float angle is returned.
    zero_2_2pi : bool, optional
        Change angle modulo range to [0, 2pi)
        Default is False.
    degree : bool, optional
        If True, then the given angles are assumed to be in degrees.
        Default is False.

    Returns
    -------
    ret : float or ndarray
        an angle or an array of modulated angle.

    Examples
    --------
    >>> angle_mod(-4.0)
    2.28318531

    >>> angle_mod([-4.0])
    np.array(2.28318531)

    >>> angle_mod([-150.0, 190.0, 350], degree=True)
    array([-150., -170.,  -10.])

    >>> angle_mod(-60.0, zero_2_2pi=True, degree=True)
    array([300.])

    """
    if isinstance(x, float):
        is_float = True
    else:
        is_float = False

    x = np.asarray(x).flatten()
    if degree:
        x = np.deg2rad(x)

    if zero_2_2pi:
        mod_angle = x % (2 * np.pi)
    else:
        mod_angle = (x + np.pi) % (2 * np.pi) - np.pi

    if degree:
        mod_angle = np.rad2deg(mod_angle)

    if is_float:
        return mod_angle.item()
    else:
        return mod_angle

class PathFinderController:
    """
    Constructs an instantiate of the PathFinderController for navigating a
    3-DOF wheeled robot on a 2D plane

    Parameters
    ----------
    Kp_rho : The linear velocity gain to translate the robot along a line
             towards the goal
    Kp_alpha : The angular velocity gain to rotate the robot towards the goal
    Kp_beta : The offset angular velocity gain accounting for smooth merging to
              the goal angle (i.e., it helps the robot heading to be parallel
              to the target angle.)
    """

    def __init__(self, Kp_rho, Kp_alpha, Kp_beta):
        self.Kp_rho = Kp_rho
        self.Kp_alpha = Kp_alpha
        self.Kp_beta = Kp_beta

    def calc_control_command(self, x_diff, y_diff, theta, theta_goal):
        """
        Returns the control command for the linear and angular velocities as
        well as the distance to goal

        Parameters
        ----------
        x_diff : The position of target with respect to current robot position
                 in x direction
        y_diff : The position of target with respect to current robot position
                 in y direction
        theta : The current heading angle of robot with respect to x axis
        theta_goal: The target angle of robot with respect to x axis

        Returns
        -------
        rho : The distance between the robot and the goal position
        v : Command linear velocity
        w : Command angular velocity
        """

        # Description of local variables:
        # - alpha is the angle to the goal relative to the heading of the robot
        # - beta is the angle between the robot's position and the goal
        #   position plus the goal angle
        # - Kp_rho*rho and Kp_alpha*alpha drive the robot along a line towards
        #   the goal
        # - Kp_beta*beta rotates the line so that it is parallel to the goal
        #   angle
        #
        # Note:
        # we restrict alpha and beta (angle differences) to the range
        # [-pi, pi] to prevent unstable behavior e.g. difference going
        # from 0 rad to 2*pi rad with slight turn

        #poo modify
        theta_inv = opposite_angle(theta)
        theta_goal_inv = opposite_angle(theta_goal)

        rho = np.hypot(x_diff, y_diff)
        alpha = angle_mod(np.arctan2(y_diff, x_diff) - theta_inv)
        beta = angle_mod(theta_goal_inv - theta_inv - alpha)
        v = -(self.Kp_rho * rho)
        w = self.Kp_alpha * alpha - self.Kp_beta * beta
        
##        rho = np.hypot(x_diff, y_diff)
##        alpha = angle_mod(np.arctan2(y_diff, x_diff) - theta)
##        beta = angle_mod(theta_goal - theta - alpha)
##        v = self.Kp_rho * rho
##        w = self.Kp_alpha * alpha - self.Kp_beta * beta

        if alpha > np.pi / 2 or alpha < -np.pi / 2:
            v = -v

        return rho, v, w


# simulation parameters
controller = PathFinderController(9, 20, 0.2)
#controller = PathFinderController(5, 5, 3)
dt = 0.01

# Robot specifications
MAX_LINEAR_SPEED = 0.5
MAX_ANGULAR_SPEED = 3

show_animation = False


def move_to_pose(x_start, y_start, theta_start, x_goal, y_goal, theta_goal,inter_dist):
    x = x_start
    y = y_start
    theta = theta_start

    x_inter,y_inter,theta_inter = cal_intermediate_point(x_goal,y_goal,theta_goal,inter_dist)

    x_diff = x_goal - x
    y_diff = y_goal - y

    x_traj, y_traj = [], []

    step = 0
    rho = np.hypot(x_diff, y_diff)
    while rho > 0.001:
        x_traj.append(x)
        y_traj.append(y)

        #goto intermidate_point 
        if (step == 0):
            x_diff = x_inter - x
            y_diff = y_inter - y
            rho, v, w = controller.calc_control_command(
                x_diff, y_diff, theta, theta_inter)
            if (rho < 0.1):
                step = 1
        #goto goal_point 
        else:
            x_diff = x_goal - x
            y_diff = y_goal - y
            rho, v, w = controller.calc_control_command(
                x_diff, y_diff, theta, theta_goal)

        if abs(v) > MAX_LINEAR_SPEED:
            v = np.sign(v) * MAX_LINEAR_SPEED

        if abs(w) > MAX_ANGULAR_SPEED:
            w = np.sign(w) * MAX_ANGULAR_SPEED

        theta = theta + w * dt
        x = x + v * np.cos(theta) * dt
        y = y + v * np.sin(theta) * dt

        

        if show_animation:  # pragma: no cover
            plt.cla()
            plt.arrow(x_start, y_start, np.cos(theta_start),
                      np.sin(theta_start), color='r', width=0.1)
            plt.arrow(x_goal, y_goal, np.cos(theta_goal),
                      np.sin(theta_goal), color='g', width=0.1)
            plot_vehicle(x, y, theta, x_traj, y_traj)


def plot_vehicle(x, y, theta, x_traj, y_traj):  # pragma: no cover
    # Corners of triangular vehicle when pointing to the right (0 radians)
    p1_i = np.array([0.5, 0, 1]).T
    p2_i = np.array([-0.5, 0.25, 1]).T
    p3_i = np.array([-0.5, -0.25, 1]).T

    T = transformation_matrix(x, y, theta)
    p1 = np.matmul(T, p1_i)
    p2 = np.matmul(T, p2_i)
    p3 = np.matmul(T, p3_i)

    plt.plot([p1[0], p2[0]], [p1[1], p2[1]], 'k-')
    plt.plot([p2[0], p3[0]], [p2[1], p3[1]], 'k-')
    plt.plot([p3[0], p1[0]], [p3[1], p1[1]], 'k-')

    plt.plot(x_traj, y_traj, 'b--')

    # for stopping simulation with the esc key.
    plt.gcf().canvas.mpl_connect(
        'key_release_event',
        lambda event: [exit(0) if event.key == 'escape' else None])

    plt.xlim(-1, 1)
    plt.ylim(0, 2)

    plt.pause(dt)


def transformation_matrix(x, y, theta):
    return np.array([
        [np.cos(theta), -np.sin(theta), x],
        [np.sin(theta), np.cos(theta), y],
        [0, 0, 1]
    ])


def main():

    for i in range(20):
        x_start = random.uniform(-0.5, 0.5)
        y_start = random.uniform(0.5, 1.5)
        #theta_start: float = 2 * np.pi * random.random() - np.pi
        #x_start = -0.75
        #y_start = 1.0
        #theta_start: float = np.pi
        x_goal = 0
        y_goal = 0
        theta_start: float = angle_mod(np.arctan2(y_start-0.5, x_start))
        theta_goal = np.pi/2
        print(f"Initial x: {round(x_start, 2)} m\nInitial y: {round(y_start, 2)} m\nInitial theta: {round(theta_start, 2)} rad\n")
        print(f"Goal x: {round(x_goal, 2)} m\nGoal y: {round(y_goal, 2)} m\nGoal theta: {round(theta_goal, 2)} rad\n")
        move_to_pose(x_start, y_start, theta_start, x_goal, y_goal, theta_goal,0.5)
    


if __name__ == '__main__':
    main()
