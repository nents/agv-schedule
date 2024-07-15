import torch
import torch.nn as nn
import torch.optim as optim
import numpy as np
import random

eps = 1e-2


class AngryBirdsEnv:
    def __init__(self, pigs):
        self.initial_pigs = pigs
        self.pigs = pigs.copy()

    def reset(self):
        self.pigs = self.initial_pigs.copy()
        return self._get_state()

    def step(self, action):
        a, b = action
        hit_pigs = []
        for pig in self.pigs:
            x, y = pig
            if abs(a * x**2 + b * x - y) < eps:  # 判断是否击中猪
                hit_pigs.append(pig)

        for pig in hit_pigs:
            self.pigs.remove(pig)

        reward = len(hit_pigs)
        done = len(self.pigs) == 0

        return self._get_state(), reward, done

    def _get_state(self):
        state = np.array(self.pigs).flatten()
        padding = np.zeros(40 - len(state))
        return torch.tensor(np.concatenate((state, padding)), dtype=torch.float32)


class DQN(nn.Module):
    def __init__(self, input_dim, output_dim):
        super(DQN, self).__init__()
        self.fc1 = nn.Linear(input_dim, 64)
        self.fc2 = nn.Linear(64, 64)
        self.fc3 = nn.Linear(64, output_dim)

    def forward(self, x):
        x = torch.relu(self.fc1(x))
        x = torch.relu(self.fc2(x))
        return self.fc3(x)


def train_dqn(
    env,
    input_dim,
    output_dim,
    num_episodes,
    batch_size,
    gamma,
    epsilon_start,
    epsilon_end,
    epsilon_decay,
):
    policy_net = DQN(input_dim, output_dim)
    target_net = DQN(input_dim, output_dim)
    target_net.load_state_dict(policy_net.state_dict())

    optimizer = optim.Adam(policy_net.parameters())
    criterion = nn.MSELoss()
    memory = []

    epsilon = epsilon_start

    for episode in range(num_episodes):
        state = env.reset()
        total_reward = 0
        done = False

        while not done:
            if random.random() < epsilon:
                action = torch.randn(output_dim)
            else:
                with torch.no_grad():
                    action = policy_net(state.unsqueeze(0)).squeeze()

            next_state, reward, done = env.step(action.numpy())
            memory.append((state, action, reward, next_state, done))
            state = next_state
            total_reward += reward

            if len(memory) > batch_size:
                batch = random.sample(memory, batch_size)
                (
                    state_batch,
                    action_batch,
                    reward_batch,
                    next_state_batch,
                    done_batch,
                ) = zip(*batch)

                state_batch = torch.stack(state_batch)
                action_batch = torch.stack(action_batch)
                reward_batch = torch.tensor(reward_batch, dtype=torch.float32)
                next_state_batch = torch.stack(next_state_batch)
                done_batch = torch.tensor(done_batch, dtype=torch.float32)

                # 计算当前Q值
                current_q_values = policy_net(state_batch)

                # 计算目标Q值
                with torch.no_grad():
                    next_q_values = target_net(next_state_batch).max(1)[0]
                    target_q_values = reward_batch + gamma * next_q_values * (
                        1 - done_batch
                    )

                # 计算损失
                loss = criterion(
                    current_q_values, target_q_values.unsqueeze(1).repeat(1, output_dim)
                )

                # 反向传播和优化
                optimizer.zero_grad()
                loss.backward()
                optimizer.step()

            if done:
                print(f"Episode {episode + 1}, Total Reward: {total_reward}")

        epsilon = max(epsilon_end, epsilon * epsilon_decay)

        if episode % 10 == 0:
            target_net.load_state_dict(policy_net.state_dict())

    return policy_net


def read_pigs_from_file(filename):
    pigs = []
    with open(filename, "r") as file:
        for line in file:
            x, y = map(float, line.strip().split())
            pigs.append((x, y))
    return pigs


def main():
    pigs = read_pigs_from_file("pigs.txt")
    env = AngryBirdsEnv(pigs)

    input_dim = 40  # 固定的状态维度
    output_dim = 2  # a 和 b

    num_episodes = 50
    batch_size = 32
    gamma = 0.7
    epsilon_start = 1.0
    epsilon_end = 0.01
    epsilon_decay = 0.995

    trained_model = train_dqn(
        env,
        input_dim,
        output_dim,
        num_episodes,
        batch_size,
        gamma,
        epsilon_start,
        epsilon_end,
        epsilon_decay,
    )

    # 使用训练好的模型进行测试
    state = env.reset()
    done = False
    num_birds = 0

    while not done:
        with torch.no_grad():
            action = trained_model(state.unsqueeze(0)).squeeze()
        state, reward, done = env.step(action.numpy())
        num_birds += 1

    print(f"Number of birds used: {num_birds}")


if __name__ == "__main__":
    main()
