import { router, Stack } from "expo-router";
import { StyleSheet, Text, TouchableOpacity, View } from "react-native";

export default function NotFoundScreen() {
  return (
    <>
      <Stack.Screen options={{ title: "Oops!" }} />
      <View style={styles.container}>
        <Text style={styles.title}>This screen doesn&apos;t exist.</Text>

        <TouchableOpacity onPress={() => router.replace('/')} style={styles.link}>
          <Text style={styles.linkText}>Go to home screen!</Text>
        </TouchableOpacity>
      </View>
    </>
  );
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    alignItems: "center",
    justifyContent: "center",
    padding: 20,
    backgroundColor: "#0a0e27",
  },
  title: {
    fontSize: 20,
    fontWeight: "bold" as const,
    color: "#fff",
  },
  link: {
    marginTop: 15,
    paddingVertical: 15,
  },
  linkText: {
    fontSize: 14,
    color: "#00bfff",
  },
});
