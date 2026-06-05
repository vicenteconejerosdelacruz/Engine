export const RightArrow = ({ active }) => {
  if(active)
    return (
      <div className={`arrow right ${active ? 'active' : ''}`}>
        <img src="signals/left-arrow.png" alt="Right Arrow" />
      </div>
    );
  else return null;
}
