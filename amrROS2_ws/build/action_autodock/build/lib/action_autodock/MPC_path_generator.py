import numpy as np
from geometry_msgs.msg import PoseStamped, Pose, Twist
from tf_transformations import euler_from_quaternion, quaternion_from_euler, quaternion_multiply
import math



class MPC_control_robot:
    def __init__(self, time_interval, prediction_horizon,no_particle, iterations):
       self.time_interval = time_interval
       self.prediction_horizon = prediction_horizon
       self.iterations = iterations
       self.no_particle = no_particle


    def objective_function(self,x):
        # Example: Sphere function
        return np.sum(x**2)

    def calculate_heading(self,pose):
            quant = pose.orientation
            orie_list = [quant.x,quant.y,quant.z,quant.w]
            (roll, pitch, yaw) = euler_from_quaternion(orie_list)
            #self.get_logger().info('current head='+ '{:.2f}'.format(yaw))
            return yaw
    def calculate_angle_distance(self,current_angle, target_angle):
            # คำนวณระยะห่างระหว่างมุมสองมุม
            delta_angle = target_angle - current_angle
            delta_angle = (delta_angle + math.pi) % (2 * math.pi) - math.pi
            #self.get_logger().info('delta_angle ='+'{:.3f}'.format(delta_angle))
            return delta_angle
        
    def calculate_dist_value(self,x,y,x_tar,y_tar):
            # คำนวณระยะห่างระหว่างจุด
            global current_pose
            dist = math.sqrt(math.pow(x-x_tar,2)+math.pow(y-y_tar,2))
            return dist

    def calculate_dist(self, target_pose):
            # คำนวณระยะห่างระหว่างจุด
            global current_pose
            dist = math.sqrt(math.pow(target_pose.position.x-current_pose.position.x,2)+math.pow(target_pose.position.y-current_pose.position.y,2))
            return dist

    def calculate_new_position_with_velocity(x_old, y_old, theta_old, linear_velocity, angular_velocity, delta_time):
            """
            คำนวณตำแหน่งใหม่ของหุ่นยนต์ AMR โดยใช้ความเร็วเชิงเส้นและความเร็วเชิงมุม
            พร้อมกับเพิ่มการรองรับความเร็วเดิมก่อนการเคลื่อนที่
            
            :param x_old: ตำแหน่งแกน X ปัจจุบัน
            :param y_old: ตำแหน่งแกน Y ปัจจุบัน
            :param theta_old: มุมการหมุนปัจจุบัน (radians)
            :param linear_velocity: ความเร็วเชิงเส้นปัจจุบัน (m/s)
            :param angular_velocity: ความเร็วเชิงมุมปัจจุบัน (rad/s)
            :param previous_linear_velocity: ความเร็วเชิงเส้นก่อนหน้า (m/s)
            :param previous_angular_velocity: ความเร็วเชิงมุมก่อนหน้า (rad/s)
            :param delta_time: ช่วงระยะเวลา (s)
            :return: ตำแหน่งใหม่ (x_new, y_new, theta_new)
            """
            
            # คำนวณความเร็วเชิงเส้นเฉลี่ยและความเร็วเชิงมุมเฉลี่ย
            #avg_linear_velocity = (linear_velocity + previous_linear_velocity) / 2
            #avg_angular_velocity = (angular_velocity + previous_angular_velocity) / 2
            avg_linear_velocity = linear_velocity
            avg_angular_velocity = angular_velocity
            # กรณีที่ avg_angular_velocity != 0 จะเป็นการเคลื่อนที่แบบโค้ง
            if avg_angular_velocity != 0:
                # คำนวณรัศมีของเส้นทางโค้ง
                R = avg_linear_velocity / avg_angular_velocity
                theta_new = theta_old + avg_angular_velocity * delta_time
                
                # คำนวณตำแหน่งใหม่
                x_new = x_old + R * (math.sin(theta_new) - math.sin(theta_old))
                y_new = y_old - R * (math.cos(theta_new) - math.cos(theta_old))
                
            else:  # กรณีที่ avg_angular_velocity = 0 จะเป็นการเคลื่อนที่ตรง
                x_new = x_old + avg_linear_velocity * delta_time * math.cos(theta_old)
                y_new = y_old + avg_linear_velocity * delta_time * math.sin(theta_old)
                theta_new = theta_old
            
            # ปรับมุม theta ให้อยู่ในช่วง -π ถึง π
            theta_new = math.atan2(math.sin(theta_new), math.cos(theta_new))
            
            return x_new, y_new, theta_new


    def cal_command(self,robot_pose, target_pose,min_vel,max_vel,min_omega,max_omega):
        self.robot_pose = robot_pose
        self.target_pose = target_pose
        self.min_vel = min_vel
        self.max_vel = max_vel
        self.min_omega = min_omega
        self.max_omega = max_omega

        self.heading_robot = calculate_heading(robot_pose)
        self.heading_target =  calculate_heading(target_pose)
        self.norm_distance = calculate_dist(self.robot_pose,self.target_pose)
        self.norm_orientation = calculate_angle_distance(self.heading_robot,self.heading_target)

        best_cmds, best_val = self.particle_swarm_optimization(objective_function, self.no_particle, self.iterations)
        vel_cmds = best_cmds[:len(best_cmds)//2]
        omega_cmds = best_cmds[len(best_cmds)//2:]

        return vel_cmds[0],omega_cmds[0]



    def objective_function(self,particle):
         #distance to target point and orientation 
        ratio_distance = 0.7
        ratio_orientation = 1 - ratio_distance
        
        vel_cmds = particle[:len(particle)//2]
        omega_cmds = particle[len(particle)//2:]

        x = self.robot_pose.position.x
        y = self.robot_pose.position.y
        head =  self.heading_robot

        for i in self.prediction_horizon:
            x,y,head = calculate_new_position_with_velocity(x, y, head, vel_cmds[i], omega_cmds[i], self.time_interval)
        
        dist = calculate_dist_value(x,y,self.target_pose.postion.x,self.target_pose.positon.y)
        head_dist = calculate_angle_distance(head, self.heading_target)

        norm_dist = dist/self.norm_distance
        norm_ang_dist = head_dist/self.norm_orientation

        return (ratio_distance*norm_dist)+(ratio_orientation*norm_ang_dist) 
    

    def particle_swarm_optimization(self, objective_func, num_particles, num_iterations):
        # Initialize particles
        dimensions = self.prediction_horizon*2  # Number of dimensions (variables) include vel and omega in each horizon
        particles_vel = np.random.uniform(low=self.min_vel, high= self.max_vel, size=(num_particles, self.prediction_horizon))
        particles_omega = np.random.uniform(low=self.min_omega, high=self.max_omega, size=(num_particles, self.prediction_horizon))
        particles = np.concatenate((particles_vel, particles_omega), axis=1) #combine particle
        print(particles)

        velocities = np.zeros((num_particles, dimensions))
        best_positions = particles.copy()
        best_values = np.array([objective_func(p) for p in particles])
        global_best_position = best_positions[np.argmin(best_values)]
        global_best_value = np.min(best_values)

        # PSO parameters
        inertia_weight = 0.7
        cognitive_weight = 1.5
        social_weight = 1.5

        for _ in range(num_iterations):
            for i in range(num_particles):
                # Update velocity
                velocities[i] = (
                    inertia_weight * velocities[i]
                    + cognitive_weight * np.random.rand() * (best_positions[i] - particles[i])
                    + social_weight * np.random.rand() * (global_best_position - particles[i])
                )
                # Update position
                particles[i] += velocities[i]
                # Clip position to bounds
                particle = particles[i]
                vel_cmds = particle[:len(particle)//2]
                omega_cmds = particle[len(particle)//2:]
                vel_cmds = np.clip(vel_cmds, self.min_vel, self.max_vel)
                omega_cmds = np.clip(omega_cmds, self.min_omega, self.max_omega)
                particles[i] = np.concatenate((vel_cmds, omega_cmds), axis=1) #combine particle
                
                # Update best positions and values
                value = objective_func(particles[i])
                if value < best_values[i]:
                    best_values[i] = value
                    best_positions[i] = particles[i]
                    if value < global_best_value:
                        global_best_value = value
                        global_best_position = particles[i]

        return global_best_position, global_best_value

        
        





if __name__ == "__main__":
    num_particles = 20
    num_iterations =  100
    best_position, best_value = particle_swarm_optimization(objective_function, num_particles, num_iterations)
    print(f"Best position: {best_position}")
    print(f"Best value: {best_value:.4f}")