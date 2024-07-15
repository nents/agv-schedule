import numpy as np
import random

eps = 0.1


def feq(x, y):
    return abs(x - y) < eps


class PigsEnv:
    def __init__(self, pigs):
        self.pigs = sorted(pigs)
        self.remaining_pigs = pigs.copy()

    def reset(self):
        self.remaining_pigs = self.pigs.copy()
        return tuple(self.remaining_pigs)

    def step(self, action):
        a, b = action
        hit_pigs = []
        for pig in self.remaining_pigs:
            x, y = pig
            if feq(a * x**2 + b * x, y):
                hit_pigs.append(pig)
        for pig in hit_pigs:
            self.remaining_pigs.remove(pig)

        # reward = len(hit_pigs) ** 2
        # reward = len(hit_pigs) * 10 - 10
        reward = len(hit_pigs) - 1
        done = len(self.remaining_pigs) == 0

        return done, tuple(self.remaining_pigs), reward


class QLearning:
    def __init__(self, action_space, gamma, learning_rate, epsilon):
        self.q_table = {}
        self.learning_rate = learning_rate
        self.gamma = gamma
        self.epsilon = epsilon
        self.action_space = action_space

    def get_action(self, state):
        if random.uniform(0, 1) < self.epsilon:
            return random.choice(self.action_space)
        if state not in self.q_table:
            return random.choice(self.action_space)
        return max(self.q_table[state], key=self.q_table[state].get)

    def update(self, state, action, reward, next_state, done):
        if state not in self.q_table:
            self.q_table[state] = {tuple(a): 0 for a in self.action_space}
        if next_state not in self.q_table:
            self.q_table[next_state] = {tuple(a): 0 for a in self.action_space}

        cur_q = self.q_table[state][tuple(action)]
        next_q = max(self.q_table[next_state].values()) if not done else 0
        new_q = cur_q + self.learning_rate * (reward + self.gamma * next_q - cur_q)
        self.q_table[state][tuple(action)] = new_q

    def evaluate(self, env):
        state = env.reset()
        done = False
        total_birds = 0
        while not done:
            action = self.get_action(state)
            done, next_state, _ = env.step(action)
            state = next_state
            total_birds += 1
        return total_birds


if __name__ == "__main__":
    # input & init setting
    pigs_file = "pigs.txt"
    pigs = []
    with open(pigs_file, "r") as f:
        for line in f:
            x, y = map(float, line.strip().split())
            pigs.append((x, y))

    env = PigsEnv(pigs)
    action_space = [
        (round(a, 2), round(b, 2))
        for a in np.arange(-10, 0, 0.1)
        for b in np.arange(0, 10, 0.1)
    ]
    agent = QLearning(
        action_space=action_space, gamma=0.9, learning_rate=0.01, epsilon=0.1
    )

    # train
    for episode in range(20000):
        state = env.reset()
        action = agent.get_action(state)
        done = False
        total_reward = 0

        while not done:
            action = agent.get_action(state)
            done, next_state, reward = env.step(action)
            agent.update(state, action, reward, next_state, done)
            state = next_state
            total_reward += reward

        if episode < 20 or episode % 4000 == 0:
            print(f"Episode {episode}: {agent.evaluate(env=env)} birds used")

    # print q
    # print(agent.q_table)

    # test
    state = env.reset()
    done = False
    total_birds = 0
    print("Test:")
    while not done:
        action = agent.get_action(state)
        print(f"#{total_birds + 1} => state={state}: action(a, b)={action}")
        done, next_state, reward = env.step(action)
        state = next_state
        total_birds += 1
    print(f"Number of birds used: {total_birds}")
